using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;

namespace BxSpi
{
    // これを利用することによって画像展開クラスや書庫展開クラスに派生できます。
	/// <summary>Susieプラグインを扱う為の基底クラスです。</summary>
	public abstract class SpiCore : IDisposable
	{
		/// <summary>Spiのハンドルを保持するポインタです。</summary>
		protected IntPtr handle = IntPtr.Zero;
		/// <summary>破棄フラグ</summary>
		protected bool disposed = false;

		/// <summary>プラグイン情報を取得するデリゲートらしい</summary>
		private SpiGetPluginInfo getPluginInfo;
		/// <summary>サポト情報を取得するデリゲートらしい</summary>
		private SpiIsSupported isSupported;

		/// <summary>メモリで確保するバッファ数</summary>
		private const int BufferSize = 128;

		/// <summary>*.spiファイルを読み込みます。</summary>
		/// <param name="fileName">読み込む*.spi</param>
		public SpiCore(string fileName)
		{
			// 指定したDLLファイルを呼び出します。
			handle = Win32.LoadLibrary(fileName);

			// デリゲートを初期化？する
			getPluginInfo = (SpiGetPluginInfo)Win32.GetProcAddress(typeof(SpiGetPluginInfo), handle, "GetPluginInfo");
			isSupported = (SpiIsSupported)Win32.GetProcAddress(typeof(SpiIsSupported), handle, "IsSupported");
		}

		/// <summary>指定したタイプの情報を取得します</summary>
		/// <param name="info">取得する列挙値</param>
		/// <returns>取得した情報</returns>
		public string GetPluginInfo(GetInfomation info)
		{
			// string用のポインタ作成
			IntPtr ptr = Marshal.AllocHGlobal(BufferSize);

			// 形式の取得
			switch (info)
			{
				case GetInfomation.ApiVersion:
					getPluginInfo(0, ptr, BufferSize);
					break;
				case GetInfomation.PluginInfo:
					getPluginInfo(1, ptr, BufferSize);
					break;
				case GetInfomation.CorrespondingType:
					getPluginInfo(2, ptr, BufferSize);
					break;
				case GetInfomation.CorrespondingTypeName:
					getPluginInfo(3, ptr, BufferSize);
					break;
			}
			// 取得した文字列を切り取る
			string str = Marshal.PtrToStringAnsi(ptr);

			// 使用しないメモリを除去
			Marshal.FreeHGlobal(ptr);

			// 文字を返す
			return str;
		}

		/// <summary>指定したファイルがSusieプラグインに対応しているかチェックします。</summary>
		/// <param name="filename">チェックするファイル</param>
		/// <returns>対応しているか</returns>
		public bool IsSupported(string fileName)
		{
			// ストリームハンドルからファイルが対応しているかチェックする。
			using (FileStream fs = new FileStream(fileName, FileMode.Open, FileAccess.Read))
				if (isSupported(fileName, fs.Handle) != 0) // fs.Handleは使用しないでください。とあるが、使用してみる。
					return true;

			// だめだったら、Falseを返す
			return false;
		}

		/// <summary>Spiの入出力情報を取得します</summary>
		public virtual ApiVersionInfomation GetApiInfo()
		{
			string str = GetPluginInfo(GetInfomation.ApiVersion);

			switch (str)
			{
				case "00IN":
					return ApiVersionInfomation._00IN;
				case "00IM":
					return ApiVersionInfomation._00IM;
				case "00XN":
					return ApiVersionInfomation._00XN;
				case "00XM":
					return ApiVersionInfomation._00XM;
				case "00AN":
					return ApiVersionInfomation._00AN;
				case "00AM":
					return ApiVersionInfomation._00AM;
				case "00CN":
					return ApiVersionInfomation._00CN;
				case "00EN":
					return ApiVersionInfomation._00EN;
				default:
					return ApiVersionInfomation.NotFound;
			}
		}

		/// <summary>プラグイン情報を取得します。</summary>
		public virtual string GetApiVersion()
		{
			return GetPluginInfo(GetInfomation.PluginInfo);

		}
		/// <summary>拡張子を取得します。</summary>
		public virtual string GetCorrespondingType()
		{
			string type = GetPluginInfo(GetInfomation.CorrespondingType);

			// 文字列結合
			StringBuilder sb = new StringBuilder();

			// 正規表現
			Regex reg = new Regex(@"(?<correspond>(\*.\w{1,}))");

			// MatchCollection で全対象を検索
			MatchCollection mc = reg.Matches(type);

			// 一致した場合は追加。
			if (mc.Count > 0)
				for (int i = 0; i < mc.Count; i++)
					if (mc[i].Groups["correspond"].Success)
					{
						string file = mc[i].Groups["correspond"].Value;

						if (file.Contains(";"))
							file.Remove(file.Length - 1);

						sb.Append(file + ";");
					}

			return sb.ToString().ToLower().Remove(sb.ToString().Length - 1);
		}

		/// <summary>拡張子の説明を返します。</summary>
		public virtual string GetCorrespondingName()
		{
			return GetPluginInfo(GetInfomation.CorrespondingTypeName);
		}

		#region 破棄

		/// <summary>Disposeが呼ばれたら破棄します。</summary>
		/// <param name="disposing">破棄フラグ</param>
		protected virtual void Dispose(bool disposing)
		{
			if (!disposed)
			{
				if (disposing)
				{
					// デリゲートを破棄します。
					getPluginInfo = null;
					isSupported = null;

					// 使用したDLLを破棄します。
					if (!Win32.FreeLibrary(handle))
						throw new Exception("Spiプラグインの破棄ができませんでした。\nハンドル : " + handle.ToString());
				}
				disposed = true;
			}
		}

		/// <summary>Disposeメソッドです。</summary>
		public void Dispose()
		{
			// 使用したDLLを破棄します。
			Dispose(true);
			GC.SuppressFinalize(this);
		}

		/// <summary>デストラクタ</summary>
		~SpiCore()
		{
			// デストラクタによる破棄は行いません。
			Dispose(false);
		}

		#endregion
	}
}
