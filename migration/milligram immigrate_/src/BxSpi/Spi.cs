using System;
using System.Runtime.InteropServices;

namespace BxSpi
{
	/* デリゲート群 */
	// <summary>Spiプラグインの情報を取得します。</summary>
	public delegate int SpiGetPluginInfo(int infono, IntPtr buf, int buflen);
	// <summary>指定したファイルがこのSpiに対応しているか。</summary>
	public delegate int SpiIsSupported([MarshalAs(UnmanagedType.LPStr)]string filename, IntPtr dw);
	// <summary>指定したファイルの画像情報を取得します。</summary>
	public delegate int SpiGetPictureInfo([MarshalAs(UnmanagedType.LPStr)]string buf, int len, uint flag, out PictureInfo lpInfo);
	// <summary>指定したファイルから画像を取得します。</summary>
	public delegate int SpiGetPicture([MarshalAs(UnmanagedType.LPStr)]string buf, int len, uint flag, out IntPtr pHBInfo, out IntPtr pHBm, IntPtr lpPrgressCallback, int lData);

	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	/// <summary>画像情報です。</summary>
	public struct PictureInfo
	{
		/// <summary>画像を展開する位置 : x</summary>
		public int left;          /* 画像を展開する位置 */
		/// <summary>画像を展開する位置 : y</summary>
		public int top;          /* 画像を展開する位置 */
		/// <summary>画像の幅です。</summary>
		public int width;             /* 画像の幅(pixel) */
		/// <summary>画像の高さです。</summary>
		public int height;            /* 画像の高さ(pixel) */
		/// <summary>画素の水平方向密度です。</summary>
		public short x_density;         /* 画素の水平方向密度 */
		/// <summary>画素の垂直方向密度です。</summary>
		public short y_density;         /* 画素の垂直方向密度 */
		/// <summary>色深度です。</summary>
		public short colorDepth;       /* 画素当たりのbit数 */
		/// <summary>画像内のテキスト情報</summary>
		public IntPtr hInfo;           /* 画像内のテキスト情報 */
	}

	/// <summary>GetPluginInfoで取得するときの列挙群</summary>
	public enum GetInfomation
	{
		/// <summary>Plug-in APIバージョン</summary>
		ApiVersion,
		/// <summary>Plugn名、バージョン及びcopyright</summary>
		PluginInfo,
		/// <summary>代表的な拡張子("*.JPG" "*.RGB;*.Q0"など)</summary>
		CorrespondingType,
		/// <summary>ファイル形式名</summary>
		CorrespondingTypeName,
	}

	/// <summary>Spiタイプ</summary>
	public enum ApiVersionInfomation
	{
		_00IN,
		_00IM,
		_00XN,
		_00XM,
		_00AN,
		_00AM,
		_00CN,
		_00EN,
		NotFound,
	}
}
