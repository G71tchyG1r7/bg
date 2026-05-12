#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <cstdio>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

#define IDR_DEFAULT_BG 256

Bitmap* loadBitmapFromResource() {
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_DEFAULT_BG), L"JPG");
    if (!hRes) { fwprintf(stderr, L"FindResource failed: %lu\n", GetLastError()); return nullptr; }

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) { fwprintf(stderr, L"LoadResource failed: %lu\n", GetLastError()); return nullptr; }

    void* pData = LockResource(hData);
    DWORD size = SizeofResource(NULL, hRes);
    fwprintf(stderr, L"Resource found, size: %lu bytes\n", size);

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hMem) { fwprintf(stderr, L"GlobalAlloc failed\n"); return nullptr; }
    void* pMem = GlobalLock(hMem);
    memcpy(pMem, pData, size);
    GlobalUnlock(hMem);

    IStream* stream = nullptr;
    HRESULT hr = CreateStreamOnHGlobal(hMem, TRUE, &stream);
    if (FAILED(hr) || !stream) { fwprintf(stderr, L"CreateStreamOnHGlobal failed: %08lx\n", hr); return nullptr; }

    Bitmap* bmp = Bitmap::FromStream(stream);
    stream->Release();

    if (!bmp) { fwprintf(stderr, L"Bitmap::FromStream returned null\n"); return nullptr; }
    Status s = bmp->GetLastStatus();
    if (s != Ok) { fwprintf(stderr, L"Bitmap::FromStream status: %d\n", (int)s); delete bmp; return nullptr; }

    fwprintf(stderr, L"Bitmap loaded ok: %ux%u\n", bmp->GetWidth(), bmp->GetHeight());
    return bmp;
}

int main() {
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    WCHAR exePath[MAX_PATH + 1];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WCHAR* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash) *(lastSlash + 1) = L'\0';
    lstrcatW(exePath, L"bg.bmp");
    fwprintf(stderr, L"Output path: %ls\n", exePath);

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Status status = Ok;
    {
        Bitmap* image = nullptr;

        if (argc >= 2) {
            std::wstring inputPath = argv[1];
            fwprintf(stderr, L"Loading from file: %ls\n", inputPath.c_str());
            image = Bitmap::FromFile(inputPath.c_str());
            if (!image || image->GetLastStatus() != Ok) {
                fwprintf(stderr, L"Error: could not load '%ls'\n", inputPath.c_str());
                LocalFree(argv);
                GdiplusShutdown(gdiplusToken);
                return 1;
            }
        } else {
            fwprintf(stderr, L"No args, loading default resource...\n");
            image = loadBitmapFromResource();
            if (!image) {
                LocalFree(argv);
                GdiplusShutdown(gdiplusToken);
                return 1;
            }
        }

        LocalFree(argv);

        CLSID bmpClsid;
        CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &bmpClsid);
        status = image->Save(exePath, &bmpClsid, NULL);
        fwprintf(stderr, L"Save status: %d\n", (int)status);
        delete image;
    }

    GdiplusShutdown(gdiplusToken);

    if (status != Ok) {
        fwprintf(stderr, L"Error: failed to save BMP (GDI+ status %d)\n", (int)status);
        return 1;
    }

    fwprintf(stderr, L"Setting wallpaper...\n");
    BOOL ok = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, exePath, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    fwprintf(stderr, L"SystemParametersInfoW: %d (err %lu)\n", ok, GetLastError());

    ExitProcess(0);
    return 0;
}