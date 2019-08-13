using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Security;
using System.Runtime.ConstrainedExecution;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Drawing;
using System.Runtime.CompilerServices;

namespace Factory
{
    // Win32で使用されるライブラリ群
    public unsafe static class Win32
    {
        public static readonly IntPtr INVALID_HANDLE_VALUE = new IntPtr(-1);

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr SendMessage(IntPtr hWnd, int msg, int wParam, int lParam);

        [DllImport("user32.dll")]
        public static extern bool SetProcessDPIAware();


        
        
        #region DllCaller

        // DLL 呼び出し
        [DllImport("kernel32")]
        public static extern IntPtr LoadLibrary(string lpLibFileName);

        // DLLを解放します。
        [DllImport("kernel32")]
        public static extern bool FreeLibrary(IntPtr hLibModule);

        // 指定したポインタの指定したメソッドを呼び出し
        [DllImport("kernel32", CharSet = CharSet.Ansi, EntryPoint = "GetProcAddress")]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

        // 指定したポインタから指定したメソッドを呼び出し
        public static Delegate GetProcAddressDelegate(Type methodType, IntPtr handle, string methodName)
        {
            IntPtr ptr;

            // アンマネージド関数の呼び出し
            ptr = GetProcAddress(handle, methodName);

            if (ptr != IntPtr.Zero)
                return Marshal.GetDelegateForFunctionPointer(ptr, methodType);

            return (null);
        }

        #endregion

        #region File

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr CreateFile(
            [MarshalAs(UnmanagedType.LPTStr)] string filename,
            [MarshalAs(UnmanagedType.U4)] EFileAccess access,
            [MarshalAs(UnmanagedType.U4)] EFileShare share,
            IntPtr securityAttributes, // optional SECURITY_ATTRIBUTES struct or IntPtr.Zero
            [MarshalAs(UnmanagedType.U4)] ECreationDisposition creationDisposition,
            [MarshalAs(UnmanagedType.U4)] EFileAttributes flagsAndAttributes,
            IntPtr templateFile);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern IntPtr CreateFileA(
            [MarshalAs(UnmanagedType.LPStr)] string filename,
            [MarshalAs(UnmanagedType.U4)] EFileAccess access,
            [MarshalAs(UnmanagedType.U4)] EFileShare share,
            IntPtr securityAttributes,
            [MarshalAs(UnmanagedType.U4)] ECreationDisposition creationDisposition,
            [MarshalAs(UnmanagedType.U4)] EFileAttributes flagsAndAttributes,
            IntPtr templateFile);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern IntPtr CreateFileW(
            [MarshalAs(UnmanagedType.LPWStr)] string filename,
            [MarshalAs(UnmanagedType.U4)] EFileAccess access,
            [MarshalAs(UnmanagedType.U4)] EFileShare share,
            IntPtr securityAttributes,
            [MarshalAs(UnmanagedType.U4)] ECreationDisposition creationDisposition,
            [MarshalAs(UnmanagedType.U4)] EFileAttributes flagsAndAttributes,
            IntPtr templateFile);

        [Flags]
        public enum EFileAccess : uint
        {
            //
            // Standart Section
            //

            ACCESS_SYSTEMSECURITY = 0x1000000,   // AccessSystemAcl access type
            MAXIMUM_ALLOWED = 0x2000000,     // MaximumAllowed access type

            DELETE = 0x10000,
            READ_CONTROL = 0x20000,
            WRITE_DAC = 0x40000,
            WRITE_OWNER = 0x80000,
            SYNCHRONIZE = 0x100000,

            STANDARD_RIGHTS_REQUIRED = 0xF0000,
            STANDARD_RIGHTS_READ = READ_CONTROL,
            STANDARD_RIGHTS_WRITE = READ_CONTROL,
            STANDARD_RIGHTS_EXECUTE = READ_CONTROL,
            STANDARD_RIGHTS_ALL = 0x1F0000,
            SPECIFIC_RIGHTS_ALL = 0xFFFF,

            FILE_READ_DATA = 0x0001,        // file & pipe
            FILE_LIST_DIRECTORY = 0x0001,       // directory
            FILE_WRITE_DATA = 0x0002,       // file & pipe
            FILE_ADD_FILE = 0x0002,         // directory
            FILE_APPEND_DATA = 0x0004,      // file
            FILE_ADD_SUBDIRECTORY = 0x0004,     // directory
            FILE_CREATE_PIPE_INSTANCE = 0x0004, // named pipe
            FILE_READ_EA = 0x0008,          // file & directory
            FILE_WRITE_EA = 0x0010,         // file & directory
            FILE_EXECUTE = 0x0020,          // file
            FILE_TRAVERSE = 0x0020,         // directory
            FILE_DELETE_CHILD = 0x0040,     // directory
            FILE_READ_ATTRIBUTES = 0x0080,      // all
            FILE_WRITE_ATTRIBUTES = 0x0100,     // all

            //
            // Generic Section
            //

            GENERIC_READ = 0x80000000,
            GENERIC_WRITE = 0x40000000,
            GENERIC_EXECUTE = 0x20000000,
            GENERIC_ALL = 0x10000000,

            FILE_ALL_ACCESS =
            STANDARD_RIGHTS_REQUIRED |
            SYNCHRONIZE |
            0x1FF,

            FILE_GENERIC_READ =
            STANDARD_RIGHTS_READ |
            FILE_READ_DATA |
            FILE_READ_ATTRIBUTES |
            FILE_READ_EA |
            SYNCHRONIZE,

            FILE_GENERIC_WRITE =
            STANDARD_RIGHTS_WRITE |
            FILE_WRITE_DATA |
            FILE_WRITE_ATTRIBUTES |
            FILE_WRITE_EA |
            FILE_APPEND_DATA |
            SYNCHRONIZE,

            FILE_GENERIC_EXECUTE =
            STANDARD_RIGHTS_EXECUTE |
            FILE_READ_ATTRIBUTES |
            FILE_EXECUTE |
            SYNCHRONIZE
        }

        [Flags]
        public enum EFileShare : uint
        {
            NULL = 0x00000000,
            FILE_SHARE_READ = 0x00000001,
            FILE_SHARE_WRITE = 0x00000002,
            FILE_SHARE_DELETE = 0x00000004
        }

        public enum ECreationDisposition : uint
        {
            CREATE_NEW = 1,
            CREATE_ALWAYS = 2,
            OPEN_EXISTING = 3,
            OPEN_ALWAYS = 4,
            TRUNCATE_EXISTING = 5
        }

        [Flags]
        public enum EFileAttributes : uint
        {
            FILE_ATTRIBUTE_READONLY = 0x00000001,
            FILE_ATTRIBUTE_HIDDEN = 0x00000002,
            FILE_ATTRIBUTE_SYSTEM = 0x00000004,
            FILE_ATTRIBUTE_DIRECTORY = 0x00000010,
            FILE_ATTRIBUTE_ARCHIVE = 0x00000020,
            FILE_ATTRIBUTE_DEVICE = 0x00000040,
            FILE_ATTRIBUTE_NORMAL = 0x00000080,
            FILE_ATTRIBUTE_TEMPORARY = 0x00000100,
            FILE_ATTRIBUTE_SPARSE_FILE = 0x00000200,
            FILE_ATTRIBUTE_REPARSE_POINT = 0x00000400,
            FILE_ATTRIBUTE_COMPRESSED = 0x00000800,
            FILE_ATTRIBUTE_OFFLINE = 0x00001000,
            FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 0x00002000,
            FILE_ATTRIBUTE_ENCRYPTED = 0x00004000,
            FILE_ATTRIVUTE_FIRST_PIPE_INSTANCE = 0x00080000,

            FILE_FLAG_SESSION_AWARE = 0x00800000,
            FILE_FLAG_WRITE_THROUGH = 0x80000000,
            FILE_FLAG_OVERLAPPED = 0x40000000,
            FILE_FLAG_NO_BUFFERING = 0x20000000,
            FILE_FLAG_RANDOM_ACCESS = 0x10000000,
            FILE_FLAG_SEQUENTIAL_SCAN = 0x08000000,
            FILE_FLAG_DELETE_ON_CLOSE = 0x04000000,
            FILE_FLAG_BACKUP_SEMANTICS = 0x02000000,
            FILE_FLAG_POSIX_SEMANTICS = 0x01000000,
            FILE_FLAG_OPEN_REPARSE_POINT = 0x00200000,
            FILE_FLAG_OPEN_NO_RECALL = 0x00100000,
        }

        [DllImport("kernel32.dll")]
        public static extern bool GetFileSizeEx(IntPtr hFile, out long lpFileSize);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr CreateFileMapping(
            IntPtr hFile,
            IntPtr lpFileMappingAttributes,
            EFileMapProtection flProtect,
            uint dwMaximumSizeHigh,
            uint dwMaximumSizeLow,
            [MarshalAs(UnmanagedType.LPStr)] string lpName);

        [Flags]
        public enum EFileMapProtection : uint
        {
            PAGE_READONLY = 0x02,
            PAGE_READWRITE = 0x04,
            PAGE_WRITECOPY = 0x08,
            PAGE_EXECUTEREAD = 0x20,
            PAGE_EXECUTEREADWRITE = 0x40,
            SEC_COMMIT = 0x8000000,
            SEC_IMAGE = 0x1000000,
            SEC_NOCACHE = 0x10000000,
            SEC_RESERVE = 0x4000000,
        }

        [DllImport("kernel32.dll")]
        public static extern IntPtr MapViewOfFile(IntPtr hFileMappingObject,
           EFileMapAccessType dwDesiredAccess, uint dwFileOffsetHigh, uint dwFileOffsetLow,
           uint dwNumberOfBytesToMap);

        [Flags]
        public enum EFileMapAccessType : uint
        {
            FILE_MAP_COPY = 0x01,
            FILE_MAP_WRITE = 0x02,
            FILE_MAP_READ = 0x04,
            FILE_MAP_ALL_ACCESS = 0x08,
            FILE_MAP_EXECUTE = 0x20,
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool UnmapViewOfFile(IntPtr lpBaseAddress);

        [DllImport("kernel32.dll", SetLastError = true)]
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
        [SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool CloseHandle(IntPtr hObject);

        #endregion

        #region Memory
        [DllImport("kernel32.dll")]
        public static extern IntPtr GlobalLock(IntPtr hMem);

        [DllImport("kernel32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool GlobalUnlock(IntPtr hMem);

        [DllImport("kernel32.dll")]
        public static extern IntPtr GlobalAlloc(EGlobalMemoryFlags uFlags, UIntPtr dwBytes);

        [DllImport("kernel32.dll")]
        public static extern IntPtr GlobalFree(IntPtr hMem);

        [Flags]
        public enum EGlobalMemoryFlags
        {
            GHND = 0x0042,
            GMEM_FIXED = 0x0000,
            GMEM_MOVEABLE = 0x0002,
            GMEM_ZEROINIT = 0x0040,
            GPTR = 0x0040,
        }


        [DllImport("kernel32.dll")]
        public static extern IntPtr LocalLock(IntPtr hMem);

        [DllImport("kernel32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool LocalUnlock(IntPtr hMem);

        [DllImport("kernel32.dll")]
        public static extern IntPtr LocalAlloc(ELocalMemoryFlags uFlags, UIntPtr dwBytes);

        [DllImport("kernel32.dll")]
        public static extern IntPtr LocalFree(IntPtr hMem);

        [Flags]
        public enum ELocalMemoryFlags
        {
            LMEM_FIXED = 0x0000,
            LMEM_MOVEABLE = 0x0002,
            LMEM_NOCOMPACT = 0x0010,
            LMEM_NODISCARD = 0x0020,
            LMEM_ZEROINIT = 0x0040,
            LMEM_MODIFY = 0x0080,
            LMEM_DISCARDABLE = 0x0F00,
            LMEM_VALID_FLAGS = 0x0F72,
            LMEM_INVALID_HANDLE = 0x8000,
            LHND = (LMEM_MOVEABLE | LMEM_ZEROINIT),
            LPTR = (LMEM_FIXED | LMEM_ZEROINIT),
            NONZEROLHND = (LMEM_MOVEABLE),
            NONZEROLPTR = (LMEM_FIXED)
        }

        [DllImport("kernel32.dll")]
        static extern void CopyMemory(IntPtr dst, IntPtr src, int size);

        [DllImport("gdi32.dll", EntryPoint = "DeleteObject")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool DeleteObject([In] IntPtr hObject);

        #endregion

        #region Bitmap

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public unsafe struct BITMAPINFOHEADER
        {
            public uint biSize;
            public int biWidth;
            public int biHeight;
            public ushort biPlanes;
            public ushort biBitCount;
            public EBitmapCompressionMode biCompression;
            public uint biSizeImage;
            public int biXPelsPerMeter;
            public int biYPelsPerMeter;
            public uint biClrUsed;
            public uint biClrImportant;
        }

        public enum EBitmapCompressionMode : uint
        {
            BI_RGB = 0,
            BI_RLE8 = 1,
            BI_RLE4 = 2,
            BI_BITFIELDS = 3,
            BI_JPEG = 4,
            BI_PNG = 5
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public unsafe struct RGBQUAD
        {
            public byte rgbBlue;
            public byte rgbGreen;
            public byte rgbRed;
            public byte rgbReserved;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public unsafe struct BITMAPINFO
        {
            public BITMAPINFOHEADER bmiHeader;
            public fixed byte bmiColors[4 * 256]; // public fixed RGBQUAD bmiColors[256] とやりたいところ
        }

        [DllImport("gdi32.dll", EntryPoint = "GetDIBits")]
        public static extern int GetDIBits([In] IntPtr hdc, [In] IntPtr hbmp, uint uStartScan, uint cScanLines, [Out] byte[] lpvBits, ref BITMAPINFO lpbi, EDIBColorMode uUsage);
        [Flags]
        public enum EDIBColorMode : uint
        {
            DIB_RGB_COLORS = 0,
            DIB_PAL_COLORS = 1
        }


        [Flags]
        public enum EDIBInitializeMode : uint
        {
            CLEAR = 0,
            CBM_INIT = 4
        }

        [DllImport("user32.dll")]
        public static extern IntPtr GetWindowDC(IntPtr hWnd);

        //public static extern bool GetDIBSizes
        [DllImport("gdi32.dll")]
        public static extern IntPtr CreateDIBSection(IntPtr hdc, [In] ref BITMAPINFO pbmi,
           Win32.EDIBColorMode pila, out IntPtr ppvBits, IntPtr hSection, uint dwOffset);

        [DllImport("gdi32.dll")]
        public static extern IntPtr CreateDIBitmap(IntPtr hdc, [In] ref BITMAPINFOHEADER lpbmih, EDIBInitializeMode fdwInit, IntPtr lpbInit, [In] ref BITMAPINFO lpbmi, EDIBColorMode fuUsage);

        [DllImport("gdi32.dll", EntryPoint = "CreateCompatibleDC", SetLastError = true)]
        public static extern IntPtr CreateCompatibleDC([In] IntPtr hdc);

        [DllImport("gdi32.dll", EntryPoint = "BitBlt", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool BitBlt([In] IntPtr hdc, int nXDest, int nYDest, int nWidth, int nHeight, [In] IntPtr hdcSrc, int nXSrc, int nYSrc, ETernaryRasterOperations dwRop);

        [Flags]
        public enum ETernaryRasterOperations : uint
        {
            SRCCOPY = 0x00CC0020,
            SRCPAINT = 0x00EE0086,
            SRCAND = 0x008800C6,
            SRCINVERT = 0x00660046,
            SRCERASE = 0x00440328,
            NOTSRCCOPY = 0x00330008,
            NOTSRCERASE = 0x001100A6,
            MERGECOPY = 0x00C000CA,
            MERGEPAINT = 0x00BB0226,
            PATCOPY = 0x00F00021,
            PATPAINT = 0x00FB0A09,
            PATINVERT = 0x005A0049,
            DSTINVERT = 0x00550009,
            BLACKNESS = 0x00000042,
            WHITENESS = 0x00FF0062,
            CAPTUREBLT = 0x40000000
        }

        [DllImport("gdi32.dll", ExactSpelling = true, SetLastError = true)]
        public static extern IntPtr SelectObject(IntPtr hdc, IntPtr hgdiobj);

        [DllImport("gdi32.dll")]
        public static extern int SetDIBitsToDevice(IntPtr hdc, int XDest, int YDest, uint
           dwWidth, uint dwHeight, int XSrc, int YSrc, uint uStartScan, uint cScanLines,
           IntPtr lpvBits, [In] ref BITMAPINFO lpbmi, Win32.EDIBColorMode fuColorUse);

        [DllImport("gdi32.dll", EntryPoint = "DeleteDC")]
        public static extern bool DeleteDC([In] IntPtr hdc); // ReleaseDC を使う？

        [DllImport("user32.dll")]
        public static extern bool ReleaseDC(IntPtr hWnd, IntPtr hDC);

        [DllImport("gdi32.dll")]
        public static extern EStretchBltMode SetStretchBltMode(IntPtr hdc, EStretchBltMode iStretchMode);

        [Flags]
        public enum EStretchBltMode : int
        {
            STRETCH_ANDSCANS = 1,
            STRETCH_ORSCANS = 2,
            STRETCH_DELETESCANS = 3,
            STRETCH_HALFTONE = 4,
        }

        [DllImport("gdi32.dll")]
        public static extern int StretchDIBits(IntPtr hdc,
                    int XDest, int YDest, int nDestWidth, int nDestHeight,
                    int XSrc, int YSrc, int nSrcWidth, int nSrcHeight,
                    IntPtr lpBits, [In] ref BITMAPINFO lpBitsInfo,
                    Win32.EDIBColorMode iUsage, ETernaryRasterOperations dwRop);

        [DllImport("gdi32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool StretchBlt(IntPtr hdcDest,
            int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
            IntPtr hdcSrc,
            int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc,
            ETernaryRasterOperations dwRop);

        [DllImport("gdi32.dll")]
        public static extern bool PlgBlt(IntPtr hdcDest, POINT[] lpPoint, IntPtr hdcSrc,
            int nXSrc, int nYSrc, int nWidth, int nHeight, IntPtr hbmMask, int xMask, int yMask);

        [DllImport("user32.dll")]
        public static extern int GetSystemMetrics(ESystemMetric smIndex);


        [Flags]
        public enum EDpiType
        {
            EFFECTIVE = 0,
            ANGULAR = 1,
            RAW = 2,
        }

        [DllImport("User32.dll")]
        public static extern IntPtr MonitorFromPoint([In]System.Drawing.Point pt, [In]uint dwFlags);

        [DllImport("Shcore.dll")]
        public static extern IntPtr GetDpiForMonitor([In]IntPtr hmonitor, [In]EDpiType dpiType, [Out]out uint dpiX, [Out]out uint dpiY);

        
        [Flags]
        public enum ESystemMetric : int
        {
            SM_CXSCREEN = 0,  // 0x00
            SM_CYSCREEN = 1,  // 0x01
            SM_CXVSCROLL = 2,  // 0x02
            SM_CYHSCROLL = 3,  // 0x03
            SM_CYCAPTION = 4,  // 0x04
            SM_CXBORDER = 5,  // 0x05
            SM_CYBORDER = 6,  // 0x06
            SM_CXDLGFRAME = 7,  // 0x07
            SM_CXFIXEDFRAME = 7,  // 0x07
            SM_CYDLGFRAME = 8,  // 0x08
            SM_CYFIXEDFRAME = 8,  // 0x08
            SM_CYVTHUMB = 9,  // 0x09
            SM_CXHTHUMB = 10, // 0x0A
            SM_CXICON = 11, // 0x0B
            SM_CYICON = 12, // 0x0C
            SM_CXCURSOR = 13, // 0x0D
            SM_CYCURSOR = 14, // 0x0E
            SM_CYMENU = 15, // 0x0F
            SM_CXFULLSCREEN = 16, // 0x10
            SM_CYFULLSCREEN = 17, // 0x11
            SM_CYKANJIWINDOW = 18, // 0x12
            SM_MOUSEPRESENT = 19, // 0x13
            SM_CYVSCROLL = 20, // 0x14
            SM_CXHSCROLL = 21, // 0x15
            SM_DEBUG = 22, // 0x16
            SM_SWAPBUTTON = 23, // 0x17
            SM_CXMIN = 28, // 0x1C
            SM_CYMIN = 29, // 0x1D
            SM_CXSIZE = 30, // 0x1E
            SM_CYSIZE = 31, // 0x1F
            SM_CXSIZEFRAME = 32, // 0x20
            SM_CXFRAME = 32, // 0x20
            SM_CYSIZEFRAME = 33, // 0x21
            SM_CYFRAME = 33, // 0x21
            SM_CXMINTRACK = 34, // 0x22
            SM_CYMINTRACK = 35, // 0x23
            SM_CXDOUBLECLK = 36, // 0x24
            SM_CYDOUBLECLK = 37, // 0x25
            SM_CXICONSPACING = 38, // 0x26
            SM_CYICONSPACING = 39, // 0x27
            SM_MENUDROPALIGNMENT = 40, // 0x28
            SM_PENWINDOWS = 41, // 0x29
            SM_DBCSENABLED = 42, // 0x2A
            SM_CMOUSEBUTTONS = 43, // 0x2B
            SM_SECURE = 44, // 0x2C
            SM_CXEDGE = 45, // 0x2D
            SM_CYEDGE = 46, // 0x2E
            SM_CXMINSPACING = 47, // 0x2F
            SM_CYMINSPACING = 48, // 0x30
            SM_CXSMICON = 49, // 0x31
            SM_CYSMICON = 50, // 0x32
            SM_CYSMCAPTION = 51, // 0x33
            SM_CXSMSIZE = 52, // 0x34
            SM_CYSMSIZE = 53, // 0x35
            SM_CXMENUSIZE = 54, // 0x36
            SM_CYMENUSIZE = 55, // 0x37
            SM_ARRANGE = 56, // 0x38
            SM_CXMINIMIZED = 57, // 0x39
            SM_CYMINIMIZED = 58, // 0x3A
            SM_CXMAXTRACK = 59, // 0x3B
            SM_CYMAXTRACK = 60, // 0x3C
            SM_CXMAXIMIZED = 61, // 0x3D
            SM_CYMAXIMIZED = 62, // 0x3E
            SM_NETWORK = 63, // 0x3F
            SM_CLEANBOOT = 67, // 0x43
            SM_CXDRAG = 68, // 0x44
            SM_CYDRAG = 69, // 0x45
            SM_SHOWSOUNDS = 70, // 0x46
            SM_CXMENUCHECK = 71, // 0x47
            SM_CYMENUCHECK = 72, // 0x48
            SM_SLOWMACHINE = 73, // 0x49
            SM_MIDEASTENABLED = 74, // 0x4A
            SM_MOUSEWHEELPRESENT = 75, // 0x4B
            SM_XVIRTUALSCREEN = 76, // 0x4C
            SM_YVIRTUALSCREEN = 77, // 0x4D
            SM_CXVIRTUALSCREEN = 78, // 0x4E
            SM_CYVIRTUALSCREEN = 79, // 0x4F
            SM_CMONITORS = 80, // 0x50
            SM_SAMEDISPLAYFORMAT = 81, // 0x51
            SM_IMMENABLED = 82, // 0x52
            SM_CXFOCUSBORDER = 83, // 0x53
            SM_CYFOCUSBORDER = 84, // 0x54
            SM_TABLETPC = 86, // 0x56
            SM_MEDIACENTER = 87, // 0x57
            SM_STARTER = 88, // 0x58
            SM_SERVERR2 = 89, // 0x59
            SM_MOUSEHORIZONTALWHEELPRESENT = 91, // 0x5B
            SM_CXPADDEDBORDER = 92, // 0x5C
            SM_DIGITIZER = 94, // 0x5E
            SM_MAXIMUMTOUCHES = 95, // 0x5F

            SM_REMOTESESSION = 0x1000, // 0x1000
            SM_SHUTTINGDOWN = 0x2000, // 0x2000
            SM_REMOTECONTROL = 0x2001, // 0x2001


            SM_CONVERTIBLESLATEMODE = 0x2003,
            SM_SYSTEMDOCKED = 0x2004,
        }

        #endregion

        #region Device

        [DllImport("User32.Dll")]
        public static extern short GetAsyncKeyState(EVirtualKey vKey);

        public enum EVirtualKey : ushort
        {
            VK_LBUTTON = 0x01,
            VK_RBUTTON = 0x02,
            VK_CANCEL = 0x03,
            VK_MBUTTON = 0x04,
            VK_XBUTTON1 = 0x05,
            VK_XBUTTON2 = 0x06,
            VK_BACK = 0x08,
            VK_TAB = 0x09,
            VK_CLEAR = 0x0C,
            VK_RETURN = 0x0D,
            VK_SHIFT = 0x10,
            VK_CONTROL = 0x11,
            VK_MENU = 0x12,
            VK_PAUSE = 0x13,
            VK_CAPITAL = 0x14,
            VK_KANA = 0x15,
            VK_JUNJA = 0x17,
            VK_FINAL = 0x18,
            VK_KANJI = 0x19,
            VK_ESCAPE = 0x1B,
            VK_CONVERT = 0x1C,
            VK_NONCONVERT = 0x1D,
            VK_ACCEPT = 0x1E,
            VK_MODECHANGE = 0x1F,
            VK_SPACE = 0x20,
            VK_PRIOR = 0x21,
            VK_NEXT = 0x22,
            VK_END = 0x23,
            VK_HOME = 0x24,
            VK_LEFT = 0x25,
            VK_UP = 0x26,
            VK_RIGHT = 0x27,
            VK_DOWN = 0x28,
            VK_SELECT = 0x29,
            VK_PRINT = 0x2A,
            VK_EXECUTE = 0x2B,
            VK_SNAPSHOT = 0x2C,
            VK_INSERT = 0x2D,
            VK_DELETE = 0x2E,
            VK_HELP = 0x2F,
            VK_0 = 0x30,
            VK_1 = 0x31,
            VK_2 = 0x32,
            VK_3 = 0x33,
            VK_4 = 0x34,
            VK_5 = 0x35,
            VK_6 = 0x36,
            VK_7 = 0x37,
            VK_8 = 0x38,
            VK_9 = 0x39,
            VK_A = 0x41,
            VK_B = 0x42,
            VK_C = 0x43,
            VK_D = 0x44,
            VK_E = 0x45,
            VK_F = 0x46,
            VK_G = 0x47,
            VK_H = 0x48,
            VK_I = 0x49,
            VK_J = 0x4A,
            VK_K = 0x4B,
            VK_L = 0x4C,
            VK_M = 0x4D,
            VK_N = 0x4E,
            VK_O = 0x4F,
            VK_P = 0x50,
            VK_Q = 0x51,
            VK_R = 0x52,
            VK_S = 0x53,
            VK_T = 0x54,
            VK_U = 0x55,
            VK_V = 0x56,
            VK_W = 0x57,
            VK_X = 0x58,
            VK_Y = 0x59,
            VK_Z = 0x5A,
            VK_LWIN = 0x5B,
            VK_RWIN = 0x5C,
            VK_APPS = 0x5D,
            VK_SLEEP = 0x5F,
            VK_NUMPAD0 = 0x60,
            VK_NUMPAD1 = 0x61,
            VK_NUMPAD2 = 0x62,
            VK_NUMPAD3 = 0x63,
            VK_NUMPAD4 = 0x64,
            VK_NUMPAD5 = 0x65,
            VK_NUMPAD6 = 0x66,
            VK_NUMPAD7 = 0x67,
            VK_NUMPAD8 = 0x68,
            VK_NUMPAD9 = 0x69,
            VK_MULTIPLY = 0x6A,
            VK_ADD = 0x6B,
            VK_SEPARATOR = 0x6C,
            VK_SUBTRACT = 0x6D,
            VK_DECIMAL = 0x6E,
            VK_DIVIDE = 0x6F,
            VK_F1 = 0x70,
            VK_F2 = 0x71,
            VK_F3 = 0x72,
            VK_F4 = 0x73,
            VK_F5 = 0x74,
            VK_F6 = 0x75,
            VK_F7 = 0x76,
            VK_F8 = 0x77,
            VK_F9 = 0x78,
            VK_F10 = 0x79,
            VK_F11 = 0x7A,
            VK_F12 = 0x7B,
            VK_F13 = 0x7C,
            VK_F14 = 0x7D,
            VK_F15 = 0x7E,
            VK_F16 = 0x7F,
            VK_F17 = 0x80,
            VK_F18 = 0x81,
            VK_F19 = 0x82,
            VK_F20 = 0x83,
            VK_F21 = 0x84,
            VK_F22 = 0x85,
            VK_F23 = 0x86,
            VK_F24 = 0x87,
            VK_NUMLOCK = 0x90,
            VK_SCROLL = 0x91,
            VK_LSHIFT = 0xA0,
            VK_RSHIFT = 0xA1,
            VK_LCONTROL = 0xA2,
            VK_RCONTROL = 0xA3,
            VK_LMENU = 0xA4,
            VK_RMENU = 0xA5,
            VK_BROWSER_BACK = 0xA6,
            VK_BROWSER_FORWARD = 0xA7,
            VK_BROWSER_REFRESH = 0xA8,
            VK_BROWSER_STOP = 0xA9,
            VK_BROWSER_SEARCH = 0xAA,
            VK_BROWSER_FAVORITES = 0xAB,
            VK_BROWSER_HOME = 0xAC,
            VK_VOLUME_MUTE = 0xAD,
            VK_VOLUME_DOWN = 0xAE,
            VK_VOLUME_UP = 0xAF,
            VK_MEDIA_NEXT_TRACK = 0xB0,
            VK_MEDIA_PREV_TRACK = 0xB1,
            VK_MEDIA_STOP = 0xB2,
            VK_MEDIA_PLAY_PAUSE = 0xB3,
            VK_LAUNCH_MAIL = 0xB4,
            VK_LAUNCH_MEDIA_SELECT = 0xB5,
            VK_LAUNCH_APP1 = 0xB6,
            VK_LAUNCH_APP2 = 0xB7,
            VK_OEM_1 = 0xBA,
            VK_OEM_PLUS = 0xBB,
            VK_OEM_COMMA = 0xBC,
            VK_OEM_MINUS = 0xBD,
            VK_OEM_PERIOD = 0xBE,
            VK_OEM_2 = 0xBF,
            VK_OEM_3 = 0xC0,
            VK_OEM_4 = 0xDB,
            VK_OEM_5 = 0xDC,
            VK_OEM_6 = 0xDD,
            VK_OEM_7 = 0xDE,
            VK_OEM_8 = 0xDF,
            VK_OEM_102 = 0xE2,
            VK_PROCESSKEY = 0xE5,
            VK_PACKET = 0xE7,
            VK_ATTN = 0xF6,
            VK_CRSEL = 0xF7,
            VK_EXSEL = 0xF8,
            VK_EREOF = 0xF9,
            VK_PLAY = 0xFA,
            VK_ZOOM = 0xFB,
            VK_NONAME = 0xFC,
            VK_PA1 = 0xFD,
            VK_OEM_CLEAR = 0xFE
        }

        [DllImport("user32")]
        public static extern int GetKeyboardType(int nTypeFlag);

        #endregion

        #region Others

        [DllImport("User32.dll")]
        public static extern uint GetWindowThreadProcessId(IntPtr hWnd, byte[] lpdwProcessId);

        [DllImport("User32.dll")]
        public static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll")]
        public static extern bool AttachThreadInput(uint idAttach, uint idAttachTo, bool fAttach);

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern long SystemParametersInfo(uint uiAction, uint uiParam, byte[] pvParam, uint fuWinIni);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool SetForegroundWindow(IntPtr hWnd);

        public static void SetAbsoluteForegroundWindow(IntPtr hWnd)
        {
            uint nTargetID, nForegroundID;
            byte[] sp_time = new byte[4];
            byte[] pid = new byte[4];
            uint SPI_GETFOREGROUNDLOCKTIMEOUT = 0x2000;
            uint SPI_SETFOREGROUNDLOCKTIMEOUT = 0x2001;

            // フォアグラウンドウィンドウを作成したスレッドのIDを取得
            nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), null);
            // 目的のウィンドウを作成したスレッドのIDを取得
            nTargetID = GetWindowThreadProcessId(hWnd, pid);

            // スレッドのインプット状態を結び付ける
            AttachThreadInput(nTargetID, nForegroundID, true);  // TRUE で結び付け

            // 現在の設定を sp_time に保存
            SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, sp_time, 0);
            // ウィンドウの切り替え時間を 0ms にする
            SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, null, 0);

            // ウィンドウをフォアグラウンドに持ってくる
            SetForegroundWindow(hWnd);

            // 設定を元に戻す
            SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, sp_time, 0);

            // スレッドのインプット状態を切り離す
            AttachThreadInput(nTargetID, nForegroundID, false);  // FALSE で切り離し
        }

        [DllImport("winmm.dll", EntryPoint = "timeGetTime")]
        public static extern uint GetTime();

        [DllImport("winmm.dll", EntryPoint = "timeBeginPeriod")]
        public static extern uint BeginPeriod(uint uMilliseconds);

        [DllImport("winmm.dll", EntryPoint = "timeEndPeriod")]
        public static extern uint EndPeriod(uint uMilliseconds);

        [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
        public static extern int SHFileOperation([In] ref SHFILEOPSTRUCT lpFileOp);

        [Flags]
        public enum EFileOPFlags : ushort
        {
            FOF_MULTIDESTFILES = 0x1,
            FOF_CONFIRMMOUSE = 0x2,
            FOF_SILENT = 0x4,
            FOF_RENAMEONCOLLISION = 0x8,
            FOF_NOCONFIRMATION = 0x10,
            FOF_WANTMAPPINGHANDLE = 0x20,
            FOF_ALLOWUNDO = 0x40,
            FOF_FILESONLY = 0x80,
            FOF_SIMPLEPROGRESS = 0x100,
            FOF_NOCONFIRMMKDIR = 0x200,
            FOF_NOERRORUI = 0x400,
            FOF_NOCOPYSECURITYATTRIBS = 0x800,
            FOF_NORECURSION = 0x1000,
            FOF_NO_CONNECTED_ELEMENTS = 0x2000,
            FOF_WANTNUKEWARNING = 0x4000,
            FOF_NORECURSEREPARSE = 0x8000
        }

        [Flags]
        public enum EFileFuncFlags : uint
        {
            FO_MOVE = 0x1,
            FO_COPY = 0x2,
            FO_DELETE = 0x3,
            FO_RENAME = 0x4
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct SHFILEOPSTRUCT
        {
            public IntPtr hwnd;
            public EFileFuncFlags wFunc;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string pFrom;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string pTo;
            public EFileOPFlags fFlags;
            [MarshalAs(UnmanagedType.Bool)]
            public bool fAnyOperationsAborted;
            public IntPtr hNameMappings;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string lpszProgressTitle;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct POINT
        {
            public int x;
            public int y;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct DROPFILES
        {
            public uint pFiles;
            public POINT pt;
            [MarshalAs(UnmanagedType.Bool)]
            public bool fNC;
            [MarshalAs(UnmanagedType.Bool)]
            public bool fWide;
        }

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern uint RegisterClipboardFormat(string lpszFormat);

        [DllImport("winmm.dll", EntryPoint = "timeBeginPeriod")]
        public static extern uint timeBeginPeriod(uint uMilliseconds);

        [DllImport("winmm.dll", EntryPoint = "timeEndPeriod")]
        public static extern uint timeEndPeriod(uint uMilliseconds);

        [DllImport("winmm.dll", EntryPoint = "timeGetTime")]
        public static extern uint timeGetTime();

        [DllImport("winmm.dll", SetLastError = true)]
        public static extern UInt32 timeGetDevCaps(ref TIMECAPS timeCaps,
            UInt32 sizeTimeCaps);

        [StructLayout(LayoutKind.Sequential)]
        public struct TIMECAPS
        {
            public UInt32 wPeriodMin;
            public UInt32 wPeriodMax;
        };
    }

    public static class Acfc
    {
        /// <summary>
        /// メモリの内容を比較する
        /// </summary>
        /// <param name="a">比較元</param>
        /// <param name="b">比較先</param>
        /// <param name="c">長さ(バイト数)</param>
        /// <returns></returns>
        public unsafe static int MemNCmp(byte* a, byte* b, int c)
        {
            int r;
            for (int i = 0; i < c; i++)
            {
                r = a[i] - b[i];
                if (r != 0) return (r);
            }
            return (0);
        }

        /// <summary>
        /// メモリの内容を比較する
        /// </summary>
        /// <param name="a">比較元</param>
        /// <param name="b">比較先</param>
        /// <param name="c">長さ(バイト数)</param>
        /// <returns></returns>
        public unsafe static int MemNCmp(byte[] a, byte* b, int c)
        {
            int r;
            for (int i = 0; i < c; i++)
            {
                r = a[i] - b[i];
                if (r != 0) return (r);
            }
            return (0);
        }

        /// <summary>
        /// メモリの内容を比較する
        /// </summary>
        /// <param name="a">比較元</param>
        /// <param name="b">比較先</param>
        /// <param name="c">長さ(バイト数)</param>
        /// <returns></returns>
        public unsafe static int MemNCmp(byte* a, byte[] b, int c)
        {
            int r;
            for (int i = 0; i < c; i++)
            {
                r = a[i] - b[i];
                if (r != 0) return (r);
            }
            return (0);
        }

        /// <summary>
        /// メモリコピーする
        /// </summary>
        /// <param name="outDest"></param>
        /// <param name="inSrc"></param>
        /// <param name="inNumOfBytes"></param>
        static unsafe void MemCpy(void* outDest, void* inSrc, uint inNumOfBytes)
        {
            // 転送先をuint幅にalignする
            const uint align = sizeof(uint) - 1;
            uint offset = (uint)outDest & align;
            // ↑ポインタは32bitとは限らないので本来このキャストはuintではダメだが、
            // 今は下位2bitだけあればいいのでこれでOK。
            if (offset != 0)
                offset = align - offset;
            offset = global::System.Math.Min(offset, inNumOfBytes);

            // 先頭の余り部分をbyteでちまちまコピー
            byte* srcBytes = (byte*)inSrc;
            byte* dstBytes = (byte*)outDest;
            for (uint i = 0; i < offset; i++)
                dstBytes[i] = srcBytes[i];

            // uintで一気に転送
            uint* dst = (uint*)((byte*)outDest + offset);
            uint* src = (uint*)((byte*)inSrc + offset);
            uint numOfUInt = (inNumOfBytes - offset) / sizeof(uint);
            for (uint i = 0; i < numOfUInt; i++)
                dst[i] = src[i];

            // 末尾の余り部分をbyteでちまちまコピー
            for (uint i = offset + numOfUInt * sizeof(uint); i < inNumOfBytes; i++)
                dstBytes[i] = srcBytes[i];
        }

        /// <summary>
        /// byte に格納されている UTF-16 の文字列のバイト数を得る
        /// </summary>
        /// <param name="src">文字列へのポインタ</param>
        /// <returns></returns>
        public unsafe static int BStrSizeUni(byte* src)
        {
            short* p = (short*)src;
            int r = 0;
            while (*p != 0) { p++; r += 2; }
            return (r);
        }

        /// <summary>
        /// src から dest に c 文字分の UTF-16 の文字列をコピーする c はバイト数
        /// </summary>
        /// <param name="src"></param>
        /// <param name="dest"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        public unsafe static int BStrCpyUni(byte[] dest, byte* src, int i = 0) //
        {
            while (src[i] != 0 || src[i + 1] != 0)
            {
                dest[i] = src[i];
                dest[i + 1] = src[i + 1];
                i += 2;
            }
            return (i);
        }

        /// <summary>
        /// src から dest に c 文字分の UTF-16 の文字列をコピーする c はバイト数
        /// </summary>
        /// <param name="src"></param>
        /// <param name="dest"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        public unsafe static int BStrCpyUni(byte[] dest, byte[] src, int i = 0) //
        {
            while (src[i] != 0 || src[i + 1] != 0)
            {
                dest[i] = src[i];
                dest[i + 1] = src[i + 1];
                i += 2;
            }
            return (i);
        }

        /// <summary>
        /// byte に格納されている UTF-16 の文字列のバイト数を得る
        /// </summary>
        /// <param name="src">文字列へのポインタ</param>
        /// <returns></returns>
        public unsafe static bool BStrCmpUni(byte* dest, byte* src) //
        {
            int i = 0;
            while (src[i] != 0 || src[i + 1] != 0)
            {
                if (dest[i] != src[i]) return (false);
                i += 2;
            }
            return (true);
        }


        /// <summary>
        /// src から dest に c 文字分の UTF-16 の文字列を比較する c はバイト数
        /// </summary>
        /// <param name="src"></param>
        /// <param name="dest"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        public unsafe static bool BStrCmpUni(byte[] dest, byte* src) //
        {
            int i = 0;
            while (src[i] != 0 || src[i + 1] != 0)
            {
                if (dest[i] != src[i]) return (false);
                i += 2;
            }
            return (true);
        }


        /// <summary>
        /// src から dest に c 文字分の UTF-16 の文字列を比較する c はバイト数
        /// </summary>
        /// <param name="src"></param>
        /// <param name="dest"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        public unsafe static bool BStrCmpUni(byte* dest, byte[] src) //
        {
            int i = 0;
            while (src[i] != 0 || src[i + 1] != 0)
            {
                if (dest[i] != src[i]) return (false);
                i += 2;
            }
            return (true);
        }


        /// <summary>
        /// src から dest に c 文字分の UTF-16 の文字列を比較する c はバイト数
        /// </summary>
        /// <param name="src"></param>
        /// <param name="dest"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        public unsafe static bool BStrCmpUni(byte[] dest, byte[] src) //
        {
            int i = 0;
            while (src[i] != 0 || src[i + 1] != 0)
            {
                if (dest[i] != src[i]) return (false);
                i += 2;
            }
            return (true);
        }
        /*
                public unsafe static int StrCpy<T>(T[] dest, T[] src, int p = 0)
                {
                    int i = -1;
                    do
                    {
                        i++;
                        dest[i + p] = src[i];
                    }
                    while (src[i] != default);
                    return (i + p);
                }
            */
        public unsafe static int StrCpy(char[] dest, char[] src, int p = 0)
        {
            int i = -1;
            do
            {
                i++;
                dest[i + p] = src[i];
            }
            while (src[i] != default);
            return (i + p);
        }

        public unsafe static int StrCpy(char[] dest, char* src, int p = 0)
        {
            int i = -1;
            do
            {
                i++;
                dest[i + p] = src[i];
            }
            while (src[i] != default);
            return (i + p);
        }

        public unsafe static int StrCpy(byte[] dest, byte[] src, int p = 0)
        {
            int i = -1;
            do
            {
                i++;
                dest[i + p] = src[i];
            }
            while (src[i] != default);
            return (i + p);
        }

        public unsafe static int StrCpy(byte[] dest, byte* src, int p = 0)
        {
            int i = -1;
            do
            {
                i++;
                dest[i + p] = src[i];
            }
            while (src[i] != default);
            return (i + p);
        }

        public unsafe static int StrNCpy(char[] dest, char[] src, int c, int p = 0)
        {
            int i = -1;
            do
            {
                i++;
                dest[i + p] = src[i];
                if (i + 1 == c)
                {
                    dest[i + 1] = default;
                    break;
                }
            }
            while (src[i] != default);
            return (i);
        }

        public unsafe static int StrNCpy(char[] dest, char* src, int c, int p = 0)
        {
            int i = -1;
            do
            {
                i++;
                dest[i + p] = src[i];
                if (i + 1 == c)
                {
                    dest[i + 1] = default;
                    break;
                }
            }
            while (src[i] != default);
            return (i);
        }

        public unsafe static int StrNCpy(byte[] dest, byte[] src, int c, int p = 0)
        {
            int i = -1;
            do
            {
                i++;
                dest[i + p] = src[i];
                if (i + 1 == c)
                {
                    dest[i + 1] = default;
                    break;
                }
            }
            while (src[i] != default);
            return (i);
        }

        public unsafe static int StrNCpy(byte[] dest, byte* src, int c, int p = 0)
        {
            int i = -1;
            do
            {
                i++;
                dest[i + p] = src[i];
                if (i + 1 == c)
                {
                    dest[i + 1] = default;
                    break;
                }
            }
            while (src[i] != default);
            return (i);
        }

        public unsafe static int StrLen(byte[] src)
        {
            int i = 0;
            while (src[i] != default) i++;
            return (i);
        }

        public unsafe static int StrLen(byte* src)
        {
            int i = 0;
            while (src[i] != default) i++;
            return (i);
        }

        public static byte[] ANSIAddNullStr(byte[] src)
        {
            byte[] r;
            r = new byte[src.Length + 1];
            StrNCpy(r, src, src.Length);
            r[src.Length] = default;
            return (r);
        }

        /// <summary>
        /// Size データを Endian によって リトルエンディアンに並べ替える
        /// </summary>
        /// <param name="Dat">並べ替えの先頭アドレス</param>
        /// <param name="Size">並べ替える範囲</param>
        /// <param name="Mode">1:LittleEndian 2:BigEndian</param>
        public unsafe static void Endian(void* Dat, int Size, int Mode)
        {
            if (Mode == 1) return;
            if (Mode == 2)
            {
                byte[] BDat = new byte[4];
                int i;

                for (i = 0; i < Size; i++)
                    BDat[i] = ((byte*)Dat)[Size - i - 1];
                for (i = 0; i < Size; i++)
                    ((byte*)Dat)[i] = BDat[i];
            }
        }

        /// <summary>
        /// ディレクトリ文字列の最後をバックスラッシュかスラッシュで終わるようにする
        /// </summary>
        /// <param name="src"></param>
        /// <returns></returns>
        public static string IncludeTrailingPathDelimiter(string src)
        {
            if (src.IndexOf('\\') >= 0)
            {
                if (src[src.Length - 1] != '\\')
                {
                    src = src + '\\';
                }
            }
            else if (src.IndexOf('/') >= 0)
            {
                if (src[src.Length - 1] != '/')
                {
                    src = src + '/';
                }
            }
            return (src);
        }

        /// <summary>
        /// ショートカットファイルからオリジナルのファイルを取得する
        /// </summary>
        /// <param name="Lnk"></param>
        /// <returns></returns>
        public static string GetFileFromLink(string Lnk)
        {
            string extension = Path.GetExtension(Lnk);
            // ファイルへのショートカットは拡張子".lnk"
            if (".lnk" == extension)
            {
                IWshRuntimeLibrary.WshShell shell = new IWshRuntimeLibrary.WshShell();
                // ショートカットオブジェクトの取得
                IWshRuntimeLibrary.IWshShortcut shortcut = (IWshRuntimeLibrary.IWshShortcut)shell.CreateShortcut(Lnk);

                // ショートカットのリンク先の取得
                string targetPath = shortcut.TargetPath.ToString();
                return (targetPath);
            }
            return (Lnk);
        }

        /// <summary>
        /// ファイルから 開業区切りで List を読み込む
        /// </summary>
        /// <param name="DestSL"></param>
        /// <param name="FileName"></param>
        /// <returns></returns>
        public static bool LoadStringList(List<string> DestSL, string FileName)
        {
            if (File.Exists(FileName) == false) return (false);
            DestSL.AddRange(File.ReadAllLines(FileName));
            return (true);
        }

        /// <summary>
        /// ○○ = △△ というファイルから Dictionary を得る
        /// </summary>
        /// <param name="FileName"></param>
        /// <returns></returns>
        public static Dictionary<string, string> CreateMapFromEqualValueFile(string FileName)
        {
            string Temp;
            if(File.Exists(FileName) == true)
            {
                try
                {
                    Temp = File.ReadAllText(FileName);
                    return (CreateMapFromEqualValue(Temp));
                }
                catch
                {
                    return (new Dictionary<string, string>());
                }
            }
            return (new Dictionary<string, string>());
        }


        public static Dictionary<string, string> CreateMapFromEqualValue(string src)
        {
            Dictionary<string, string> resultMap = new Dictionary<string, string>();
            string[] TempStrings = src.Split('\n');

            for (int i = 0; i < TempStrings.Length; i++)
            {
                int p = TempStrings[i].IndexOf('=');
                if (p < 0) continue;

                string kstr = TempStrings[i].Substring(0, p).Trim();
                string vstr = TempStrings[i].Substring(p + 1, TempStrings[i].Length - p - 1).Trim();

                if(resultMap.ContainsKey(kstr) == false)
                   resultMap.Add(kstr, vstr);
            }

            return (resultMap);
        }

        public static string GetStringValue(Dictionary<string, string> map, string Key, string Src)
        {
            string Temp;
            if (map.TryGetValue(Key, out Temp) == false) return (Src);
            return (Temp);
        }

        public static int GetIntegerValue(Dictionary<string, string> map, string Key, int Src, int Low, int Hi)
        {
            string Temp;
            if (map.TryGetValue(Key, out Temp) == false) return (Src);

            int res = StrToInt(Temp);

            if(Low < Hi)
            {
                if (Low > res) res = Low;
                if (Hi < res) res = Hi;
            }
            return (res);
        }

        public static double GetDoubleValue(Dictionary<string, string> map, string Key, double Src, double Low, double Hi)
        {
            string Temp;
            if (map.TryGetValue(Key, out Temp) == false) return (Src);

            double res = StrToDouble(Temp);

            if (Low < Hi)
            {
                if (Low > res) res = Low;
                if (Hi < res) res = Hi;
            }
            return (res);
        }

        public static bool GetBoolValue(Dictionary<string, string> map, string Key, bool Src)
        {
            string Temp;

            if (map.TryGetValue(Key, out Temp) == false) return (Src);
            if (Temp.ToLower() == "true") return (true);
            if (Temp.ToLower() == "false") return (false);
            return (Src);
        }


        public static uint StrToUInt(string src)
        {
            uint r = 0;
            for (int i = 0; i < src.Length; i++)
            {
                if (src[i] >= '0' && src[i] <= '9')
                {
                    r = r * 10 + src[i] - '0';
                }
            }
            return (r);
        }

        public static int StrToInt(string src)
        {
            int r = 0, b = 1;
            bool first = true;
            for (int i = 0; i < src.Length; i++)
            {
                if (src[i] == '-' && first) b = -1;
                else if (src[i] >= '0' && src[i] <= '9')
                {
                    r = r * 10 + src[i] - '0';
                    first = false;
                }
            }
            return (r * b);
        }


        public static double StrToDouble(string src)
        {
            double r = 0, k = 1, b = 1;
            bool first = true;
            bool dot = false;
            for (int i = 0; i < src.Length; i++)
            {
                if (src[i] == '-' && first) b = -1;
                else if (dot)
                {
                    if (src[i] >= '0' && src[i] <= '9')
                    {
                        k /= 10.0;
                        r = r + k * (src[i] - '0');
                        first = false;
                    }
                }
                else if (src[i] >= '0' && src[i] <= '9')
                {
                    r = r * 10 + src[i] - '0';
                    first = false;
                }
                else if(src[i] == '.')
                {
                    dot = true;
                    first = false;
                }
            }
            return (r * b);
        }

        /// <summary>
        /// ディレクトリに使えない文字を取り除く
        /// </summary>
        /// <param name="Str"></param>
        /// <param name="CanBackSlash"></param>
        /// <returns></returns>
        public static string CorrectDirString(string Str, bool CanBackSlash)
        {
            string RStr = "";
            int L = Str.Length;
            int i;
            for (i = 1; i <= L; i++)
            {
                if (Str[i] != '/'
                && Str[i] != ':'
                && Str[i] != ','
                && Str[i] != ';'
                && Str[i] != '*'
                && Str[i] != '?'
                && Str[i] != '"'
                && Str[i] != '<'
                && Str[i] != '>'
                && (Str[i] != '\\' || CanBackSlash == true)
                && Str[i] != '|')
                {
                    RStr = RStr + Str[i];
                }
            }
            return (RStr);
        }


        public static bool FitsMask(string fileName, string fileMask)
        {
            string pattern =
                 '^' +
                 Regex.Escape(fileMask.Replace(".", "__DOT__")
                                      .Replace("*", "__STAR__")
                                      .Replace("?", "__QM__"))
                                      .Replace("__DOT__", "[.]")
                                      .Replace("__STAR__", ".*")
                                      .Replace("__QM__", ".")
                                    + '$';
            return new Regex(pattern, RegexOptions.IgnoreCase).IsMatch(fileName);
        }

        public static bool FileMasks(string fileName, string fileMasks)
        {
            string[] temp = fileMasks.Split(';');
            foreach(string a in temp)
            {
                if (FitsMask(fileName, a)) return (true);
            }
            return (false);
        }

        public static string GetShortFileName(string Temp, int Count)
        {
            if (Count == -1) return (Temp);
            int i, c = Count + 1;
            i = Temp.Length - 1;

            while (i > 0)
            {
                if (Temp[i] == '\\')
        		{
                    c--;
                    if (c == 0) break;
                }
                i--;
            }

            if (i > 1) i++;
            Temp = Temp.Substring(i, Temp.Length - i + 1);
            return (Temp);
        }

        /// <summary>
        /// DateTime から time_t に変換
        /// </summary>
        /// <param name="dt"></param>
        /// <returns></returns>
        public static long ToUnixTime(DateTime dt)
        {
            var dto = new DateTimeOffset(dt.Ticks, new TimeSpan(+09, 00, 00));
            return dto.ToUnixTimeSeconds();
        }

        /// <summary>
        /// time_t から DateTime に変換
        /// </summary>
        /// <param name="unixTime"></param>
        /// <returns></returns>
        public static DateTime FromUnixTime(long unixTime)
        {
            return DateTimeOffset.FromUnixTimeSeconds(unixTime).LocalDateTime;
        }


        // ファイルをゴミ箱に移動する
        public unsafe static bool DeleteFileToRecycle(string DelFile, bool Confirm, IntPtr Handle)
        {
            bool Result;
            Win32.SHFILEOPSTRUCT sfs = new Win32.SHFILEOPSTRUCT();

            DelFile = DelFile + '\0';

            sfs.fFlags = Win32.EFileOPFlags.FOF_NOERRORUI | Win32.EFileOPFlags.FOF_SIMPLEPROGRESS | Win32.EFileOPFlags.FOF_ALLOWUNDO | Win32.EFileOPFlags.FOF_WANTNUKEWARNING;

            if (!Confirm)
                sfs.fFlags |= Win32.EFileOPFlags.FOF_NOCONFIRMATION;

            sfs.hwnd = Handle;
            sfs.wFunc = Win32.EFileFuncFlags.FO_DELETE;
            sfs.pFrom = DelFile;
            sfs.pTo = default;

            Result = (Win32.SHFileOperation(ref sfs) == 0);

            return (Result);
        }

        public unsafe static bool DeleteFileToRecycle(List<string> DelFiles, bool Confirm)
        {
            bool Result;
            Win32.SHFILEOPSTRUCT sfs = new Win32.SHFILEOPSTRUCT();
            string From = "";
            if (DelFiles.Count == 0) return (true);

            for(int i = 0;i < DelFiles.Count;i++)
            {
                From = From + DelFiles[i] + '\0';
            }
            From = From + '\0';

            sfs.fFlags = Win32.EFileOPFlags.FOF_NOERRORUI | Win32.EFileOPFlags.FOF_SIMPLEPROGRESS | Win32.EFileOPFlags.FOF_ALLOWUNDO | Win32.EFileOPFlags.FOF_WANTNUKEWARNING;

            if (!Confirm)
                sfs.fFlags |= Win32.EFileOPFlags.FOF_NOCONFIRMATION;

            if (DelFiles.Count > 1)
                sfs.fFlags |= Win32.EFileOPFlags.FOF_MULTIDESTFILES;

            sfs.hwnd = IntPtr.Zero;
            sfs.wFunc = Win32.EFileFuncFlags.FO_DELETE;
            sfs.pFrom = From;
            sfs.pTo = default;

            Result = (Win32.SHFileOperation(ref sfs) == 0);

            return (Result);
        }

        public static bool CreateLink(string SrcFileName, string LinkName, string Param, string Description)
        {
            //作成するショートカットのパス
            string shortcutPath = LinkName;
            //ショートカットのリンク先
            string targetPath = SrcFileName;

            //WshShellを作成
            IWshRuntimeLibrary.WshShell shell = new IWshRuntimeLibrary.WshShell();
            //ショートカットのパスを指定して、WshShortcutを作成
            IWshRuntimeLibrary.IWshShortcut shortcut =
                (IWshRuntimeLibrary.IWshShortcut)shell.CreateShortcut(shortcutPath);
            //リンク先
            shortcut.TargetPath = targetPath;
            //コマンドパラメータ 「リンク先」の後ろに付く
            shortcut.Arguments = Param;
            //作業フォルダ
            shortcut.WorkingDirectory = Path.GetDirectoryName(SrcFileName);
            //ショートカットキー（ホットキー）
            shortcut.Hotkey = "";
            //実行時の大きさ 1が通常、3が最大化、7が最小化
            shortcut.WindowStyle = 1;
            //コメント
            shortcut.Description = Description;
            //アイコンのパス 自分のEXEファイルのインデックス0のアイコン
            shortcut.IconLocation = SrcFileName + ",0";

            //ショートカットを作成
            shortcut.Save();

            //後始末
            Marshal.FinalReleaseComObject(shortcut);
            Marshal.FinalReleaseComObject(shell);

            return (true);

        }

        public static int CheckFileIrregularChar(string src)
        {
            int i, j;
            char[] badChar = Path.GetInvalidPathChars();
            for(i = 0;i < src.Length;i++)
                for(j = 0;j < badChar.Length;j++)
                {
                    if (src[i] == badChar[j]) return (i);
                }

            return (-1);
        }
        #endregion

        public static string GetEastToSeeString(int src, int keta, string space)
        {
            string[] kstr = {"", "K", "M", "G", "T", "P", "E", "Z", "Y"};
            string res = "";

            int v = src;
            int k = 0;
            int ss = keta;
            int j;
            while(v > 0)
            {
                v = v / 10;
                k++;
                ss--;
            }

            src = ShishaGonyu(src, k - keta);

            j = k % 3;
            k = (k - 1) / 3;


            res = src.ToString().Substring(0, keta) + space + kstr[k];
            if (j != 0) res = res.Insert(j, ".");
            return (res);
        }

        public static int ShishaGonyu(int src, int keta)
        {
            int s = src, k = keta, p;
            if (keta < 0) return (src);
            while(k > 1)
            {
                s /= 10;
                k--;
            }
            p = s % 10;
            if (p >= 5) s += 10;
            s /= 10;
            k = 0;
            while(k < keta)
            {
                s *= 10;
                k++;
            }
            return (s);
        }

        public static Rectangle GetDesktopRectangle()
        {
            Rectangle r = new Rectangle(0, 0, 0, 0);
            Screen[] screens = Screen.AllScreens;
            foreach (var a in screens)
            {
                if (r.Y > a.Bounds.Y) r.Y = a.Bounds.Y;
                if (r.X + r.Width < a.Bounds.X + a.Bounds.Width) r.Width = (a.Bounds.X + a.Bounds.Width) - r.X;
                if (r.X > a.Bounds.X) r.X = a.Bounds.X;
                if (r.Y + r.Height < a.Bounds.Y + a.Bounds.Height) r.Height = (a.Bounds.Y + a.Bounds.Height) - r.Y;
            }
            return (r);
        }

        public static Rectangle GetMonitorParameter()
        {
            Rectangle r = new Rectangle(0, 0, 0, 0);
            Screen[] screens = Screen.AllScreens;
            foreach (var a in screens)
            {
                if (r.Y > a.Bounds.Y) r.Y = a.Bounds.Y;
                if (r.X + r.Width < a.Bounds.X + a.Bounds.Width) r.Width = (a.Bounds.X + a.Bounds.Width) - r.X;
                if (r.X > a.Bounds.X) r.X = a.Bounds.X;
                if (r.Y + r.Height < a.Bounds.Y + a.Bounds.Height) r.Height = (a.Bounds.Y + a.Bounds.Height) - r.Y;
            }
            return (r);
        }

    }


    /// <summary>
    /// フォルダを選択するダイアログのクラス
    /// </summary>
    public class FolderSelectDialog
    {
        public string Path { get; set; }
        public string Title { get; set; }
        public string Filter { get; set; }

        public System.Windows.Forms.DialogResult ShowDialog()
        {
            return ShowDialog(IntPtr.Zero);
        }

        public System.Windows.Forms.DialogResult ShowDialog(System.Windows.Forms.IWin32Window owner)
        {
            return ShowDialog(owner.Handle);
        }

        public System.Windows.Forms.DialogResult ShowDialog(IntPtr owner)
        {
            var dlg = new FileOpenDialogInternal() as IFileOpenDialog;
            try
            {
                dlg.SetOptions(FOS.FOS_PICKFOLDERS | FOS.FOS_FORCEFILESYSTEM);

                IShellItem item;
                if (!string.IsNullOrEmpty(this.Path))
                {
                    IntPtr idl;
                    uint atts = 0;
                    if (NativeMethods.SHILCreateFromPath(this.Path, out idl, ref atts) == 0)
                    {
                        if (NativeMethods.SHCreateShellItem(IntPtr.Zero, IntPtr.Zero, idl, out item) == 0)
                        {
                            dlg.SetFolder(item);
                        }
                    }
                }

                if (!string.IsNullOrEmpty(this.Title))
                    dlg.SetTitle(this.Title);

                var hr = dlg.Show(owner);
                if (hr.Equals(NativeMethods.ERROR_CANCELLED))
                    return System.Windows.Forms.DialogResult.Cancel;
                if (!hr.Equals(0))
                    return System.Windows.Forms.DialogResult.Abort;

                dlg.GetResult(out item);
                string outputPath;
                item.GetDisplayName(SIGDN.SIGDN_FILESYSPATH, out outputPath);
                this.Path = outputPath;

                return System.Windows.Forms.DialogResult.OK;
            }
            finally
            {
                Marshal.FinalReleaseComObject(dlg);
            }
        }

        [ComImport]
        [Guid("DC1C5A9C-E88A-4dde-A5A1-60F82A20AEF7")]
        private class FileOpenDialogInternal
        {
        }

        // not fully defined と記載された宣言は、支障ない範囲で端折ってあります。
        [ComImport]
        [Guid("42f85136-db7e-439c-85f1-e4075d135fc8")]
        [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
        private interface IFileOpenDialog
        {
            [PreserveSig]
            UInt32 Show([In] IntPtr hwndParent);
            void SetFileTypes();     // not fully defined
            void SetFileTypeIndex();     // not fully defined
            void GetFileTypeIndex();     // not fully defined
            void Advise(); // not fully defined
            void Unadvise();
            void SetOptions([In] FOS fos);
            void GetOptions(); // not fully defined
            void SetDefaultFolder(); // not fully defined
            void SetFolder(IShellItem psi);
            void GetFolder(); // not fully defined
            void GetCurrentSelection(); // not fully defined
            void SetFileName();  // not fully defined
            void GetFileName();  // not fully defined
            void SetTitle([In, MarshalAs(UnmanagedType.LPWStr)] string pszTitle);
            void SetOkButtonLabel(); // not fully defined
            void SetFileNameLabel(); // not fully defined
            void GetResult(out IShellItem ppsi);
            void AddPlace(); // not fully defined
            void SetDefaultExtension(); // not fully defined
            void Close(); // not fully defined
            void SetClientGuid();  // not fully defined
            void ClearClientData();
            void SetFilter(); // not fully defined
            void GetResults(); // not fully defined
            void GetSelectedItems(); // not fully defined
        }

        [ComImport]
        [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
        [Guid("43826D1E-E718-42EE-BC55-A1E261C37BFE")]
        private interface IShellItem
        {
            void BindToHandler(); // not fully defined
            void GetParent(); // not fully defined
            void GetDisplayName([In] SIGDN sigdnName, [MarshalAs(UnmanagedType.LPWStr)] out string ppszName);
            void GetAttributes();  // not fully defined
            void Compare();  // not fully defined
        }

        private enum SIGDN : uint // not fully defined
        {
            SIGDN_FILESYSPATH = 0x80058000,
        }

        [Flags]
        private enum FOS // not fully defined
        {
            FOS_FORCEFILESYSTEM = 0x40,
            FOS_PICKFOLDERS = 0x20,
        }

        private class NativeMethods
        {
            [DllImport("shell32.dll")]
            public static extern int SHILCreateFromPath([MarshalAs(UnmanagedType.LPWStr)] string pszPath, out IntPtr ppIdl, ref uint rgflnOut);

            [DllImport("shell32.dll")]
            public static extern int SHCreateShellItem(IntPtr pidlParent, IntPtr psfParent, IntPtr pidl, out IShellItem ppsi);

            public const uint ERROR_CANCELLED = 0x800704C7;
        }

    }
}
