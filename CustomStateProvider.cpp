#include "stdafx.h"
#include "CustomStateProvider.h"
#include "OneSyncProvider.h"
#include "Logger.h"
#include "NavSyncQueue.h"
#include <winrt/Windows.Storage.Provider.h>

namespace winrt::OneSync::implementation
{
    using namespace Windows::Storage::Provider;
    using namespace Windows::Foundation::Collections;

    IIterable<StorageProviderItemProperty> CustomStateProvider::GetItemProperties(
        hstring const& itemPath)
    {
        std::wstring path(itemPath.c_str());
        LOG_MODULE_DEBUG(L"CustomState", L"GetItemProperties called for: %s", path.c_str());

        // 导航触发同步：推入目录路径到进程内队列
        try
        {
            auto* mode = OneSyncProvider::FindModeByPath(path);
            if (mode)
            {
                // 提取相对路径
                std::wstring relativePath;
                if (path.size() > mode->clientFolder.size())
                {
                    relativePath = path.substr(mode->clientFolder.size());
                    if (!relativePath.empty() && (relativePath[0] == L'\\' || relativePath[0] == L'/'))
                        relativePath = relativePath.substr(1);
                }

                // 提取目录部分（去掉文件名）
                std::wstring dirPath = relativePath;
                size_t lastSlash = dirPath.find_last_of(L"\\/");
                if (lastSlash != std::wstring::npos)
                    dirPath = dirPath.substr(0, lastSlash);

                LOG_MODULE_INFO(L"CustomState", L"Navigation-triggered directory sync via IPC: %s (mode: %s)",
                    dirPath.c_str(), mode->modeName.c_str());

                // 通过 IPC 推入目录路径
                NavSyncQueue::Push(mode->modeName, dirPath);
            }
        }
        catch (...)
        {
            LOG_MODULE_WARN(L"CustomState", L"Navigation-triggered sync via IPC failed: 0x%08x",
                static_cast<HRESULT>(winrt::to_hresult()));
        }

        try
        {
            // Get file attributes to determine status
            DWORD attrs = GetFileAttributesW(path.c_str());
            if (attrs == INVALID_FILE_ATTRIBUTES)
            {
                LOG_MODULE_DEBUG(L"CustomState", L"Cannot get attributes for: %s", path.c_str());
                return single_threaded_vector<StorageProviderItemProperty>();
            }

            bool isPinned = false;
            bool isInSync = false;
            bool isOnlineOnly = false;

            // Check placeholder state via CfGetPlaceholderInfo
            HANDLE hFile = CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                CF_PLACEHOLDER_BASIC_INFO info = {};
                HRESULT hr = CfGetPlaceholderInfo(hFile, CF_PLACEHOLDER_INFO_BASIC, &info, sizeof(info), nullptr);
                if (SUCCEEDED(hr))
                {
                    isPinned = (info.PinState == CF_PIN_STATE_PINNED);
                    isInSync = (info.InSyncState == CF_IN_SYNC_STATE_IN_SYNC);
                }
                CloseHandle(hFile);
            }

            // Check if file is online-only (dehydrated)
            isOnlineOnly = (attrs & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS) != 0;
            bool isSparse = (attrs & FILE_ATTRIBUTE_SPARSE_FILE) != 0;

            LOG_MODULE_DEBUG(L"CustomState", L"State for %s: isPinned=%d isInSync=%d isOnlineOnly=%d isSparse=%d attrs=0x%08x",
                path.c_str(), (int)isPinned, (int)isInSync, (int)isOnlineOnly, (int)isSparse, attrs);

            // Determine the primary status to display
            auto properties = single_threaded_vector<StorageProviderItemProperty>();

            StorageProviderItemProperty prop;

            // REQUIRED: Set icon resource to prevent Explorer crash (0xC0000005 in windows.storage.dll)
            // Without this, Explorer crashes when displaying file status icons for Cloud Files placeholders
            prop.IconResource(L"shell32.dll,-277");

            // 状态判断优先级（修复后）：
            // 1. PINNED + 正在水化（sparse）→ 显示"同步中"（用户点击了"始终保留"但水化未完成）
            // 2. PINNED + 已水化（非sparse）+ IN_SYNC → 显示"Pinned"（水化完成，已同步）
            // 3. PINNED + 已水化（非sparse）+ NOT_IN_SYNC → 显示"同步中"（水化完成但未同步）
            // 4. OnlineOnly（未钉选，仅在线）→ 显示"OnlineOnly"
            // 5. 未钉选 + NOT_IN_SYNC → 显示"Syncing"（本地修改待上传）
            // 6. 已同步 → 显示"Synced"
            if (isPinned && isSparse)
            {
                // PINNED 但仍脱水（正在水化中）→ 显示"同步中"
                prop.Id(2);  // Syncing
                prop.Value(L"Syncing");
                properties.Append(prop);
            }
            else if (isPinned && !isSparse && isInSync)
            {
                // PINNED + 已水化 + IN_SYNC → 完全同步，显示"Pinned"
                prop.Id(4);  // Pinned
                prop.Value(L"Pinned");
                properties.Append(prop);
            }
            else if (isPinned && !isSparse && !isInSync)
            {
                // PINNED + 已水化 + NOT_IN_SYNC → 水化完成但同步状态未更新，显示"同步中"
                prop.Id(2);  // Syncing
                prop.Value(L"Syncing");
                properties.Append(prop);
            }
            else if (isOnlineOnly)
            {
                // File is online-only (not pinned, data not local)
                prop.Id(3);  // OnlineOnly
                prop.Value(L"OnlineOnly");
                properties.Append(prop);
            }
            else if (!isInSync)
            {
                // File is being synced (modified locally, not yet uploaded)
                prop.Id(2);  // Syncing
                prop.Value(L"Syncing");
                properties.Append(prop);
            }
            else
            {
                // File is synced and available
                prop.Id(1);  // Synced
                prop.Value(L"Synced");
                properties.Append(prop);
            }

            return properties;
        }
        catch (...)
        {
            LOG_MODULE_ERROR(L"CustomState", L"GetItemProperties exception: 0x%08x",
                static_cast<HRESULT>(winrt::to_hresult()));
            return single_threaded_vector<StorageProviderItemProperty>();
        }
    }
}
