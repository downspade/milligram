using System;
using System.Runtime.InteropServices;

namespace BxSpi
{
    /// <summary>Win32で使用されるライブラリ群</summary>
	public static class Win32
    {
        #region DllCaller

        /// <summary>指定したDLLを呼び出します。</summary>
		/// <param name="lpLibFileName">呼び出すDLLファイル</param>
		/// <returns>ポインタ</returns>
		[DllImport("kernel32")]
		public static extern IntPtr LoadLibrary(string lpLibFileName);

		/// <summary>呼び出したDLLを解放します。</summary>
		/// <param name="hLibModule">解放するハンドル</param>
		/// <returns>破棄できたか</returns>
		[DllImport("kernel32")]
		public static extern bool FreeLibrary(IntPtr hLibModule);

		/// <summary>指定したポインタの指定したメソッドを呼び出します。</summary>
		/// <param name="hModule">呼び出すポインタ</param>
		/// <param name="lpProcName">呼び出すメソッド名</param>
		/// <returns>ポインタ</returns>
        [DllImport("kernel32", CharSet = CharSet.Ansi, EntryPoint = "GetProcAddress")]
		private static extern IntPtr getProcAddress(IntPtr hModule, string lpProcName);
        		
		/// <summary>指定したポインタから指定したメソッドを呼び出します。</summary>
		/// <param name="methodName">呼び出すメソッド名</param>
		/// <returns>delegateさん</returns>
		public static Delegate GetProcAddress(Type methodType, IntPtr handle, string methodName)
		{
            IntPtr ptr;

            // アンマネージド関数の呼び出し
            ptr = getProcAddress(handle, methodName);

            if (ptr != IntPtr.Zero)
                return Marshal.GetDelegateForFunctionPointer(ptr, methodType);

            return null;
        }

        #endregion

        #region BitmapHeader's

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct BITMAPINFOHEADER
        {
            public uint biSize;
            public int biWidth;
            public int biHeight;
            public ushort biPlanes;
            public ushort biBitCount;
            public uint biCompression;
            public uint biSizeImage;
            public int biXPelsPerMeter;
            public int biYPelsPerMeter;
            public uint biClrUsed;
            public uint biClrImportant;
        }
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct RGBQUAD
        {
            public byte rgbBlue;
            public byte rgbGreen;
            public byte rgbRed;
            public byte rgbReserved;
        }
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct BITMAPINFO
        {
            public BITMAPINFOHEADER bmiHeader;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 256)]
            public RGBQUAD[] bmiColors;
        }

        #endregion
    }
}
