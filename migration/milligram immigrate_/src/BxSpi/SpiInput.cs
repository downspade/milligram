using System;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;
using nicoAE.Plugin.Interface;
using nicoAE.Plugin.Utils;

namespace BxSpi
{
	/// <summary>Susieプラグインを利用するクラス</summary>
	public class SpiInput : IMediaInput
	{
		private string filename = "";
		private string correspondingTypes = "";
		private string[] spi = Directory.GetFiles(Application.StartupPath + @"\plugins", "*.spi", SearchOption.AllDirectories);
		private IHost IH = null;
		private Bitmap bmp = null;

		public SpiInput()
		{
			if (correspondingTypes == "")
			{
				if (spi.Length == 0)
				{
					return;
				}

				// 拡張子連結用
				StringBuilder sb = new StringBuilder();
				StringBuilder buf = new StringBuilder();
			
				// あったらリストに追加。
				for (int i = 0; i < spi.Length; i++)
				{
					using (SpiPicture si = new SpiPicture(spi[i]))
					{
						if (si.GetApiInfo() != ApiVersionInfomation._00IN) continue;

						buf.Append(si.GetCorrespondingType() + ";");

						sb.Append(si.GetCorrespondingName() + "|" + si.GetCorrespondingType() + "|");
					}
				}

				// 拡張子を連結します。
				correspondingTypes = "開けるファイル (" + buf.ToString().Remove(buf.ToString().Length - 1) + ")|" + buf.ToString().Remove(buf.ToString().Length - 1) + "|" + sb.ToString().Remove(sb.ToString().Length - 1);
			}
		}

		/// <summary>プラグイン名です。</summary>
		public string PluginName
		{
			get { return "SusiePluginInput"; }
		}

		/// <summary>ダイアログで列挙される拡張子</summary>
		public string CorrespondingType
		{
			get { return correspondingTypes; }
		}

		/// <summary>ファイル名です。</summary>
		public string FileName
		{
			get { return filename; }
		}

		/// <summary>指定位置のフレームを取得します。</summary>
		public Bitmap GetFrame(int index)
		{
			return (Bitmap)bmp.Clone();
		}

		/// <summary>入力ファイルのプロジェクト上での長さを取得します。</summary>
		public int GetLength()
		{
			return (int)Math.Round(5 * IH.ProjectFrameRate, 0, MidpointRounding.AwayFromZero);
		}

		/// <summary>入力ファイルの情報を取得します。</summary>
		public MediaInfo GetMediaInfo()
		{
			return new MediaInfo(1, bmp.Size, MediaType.Image, 0);
		}

		/// <summary>プラグインを初期化します。</summary>
		public void Initialize(IHost host)
		{
			IH = host;
		}

		/// <summary>ファイルを読み込みます。</summary>
		public bool LoadFile(string file)
		{
			filename = file;

			if (spi.Length == 0)
			{
				// プラグインが一つもない場合はエラー表示
				MessageBox.Show("プラグインがインストールもしくは、展開されていません。", "プラグインエラー", MessageBoxButtons.OK, MessageBoxIcon.Information);

				// だめだったのよ～
				return false;
			}

			try
			{
				for (int i = 0; i < spi.Length; i++)
				{
					// 一回ごとに破棄する方が効率がいいと思う。
					using (SpiPicture si = new SpiPicture(spi[i]))
					{
						// プラグインが画像入力に対応しているか。
						if (si.GetApiInfo() != ApiVersionInfomation._00IN) continue;
											
						// 入力画像がサポートされているか。
						if (!si.IsSupported(file)) continue;

						// null値を確認するための一時バッファ
						Bitmap buf = si.GetPicture(file);

						// ぬるぽ
						if (buf == null) return false;

						// 画像を32bitに変換
						bmp = PluginUtils.ConvertBitmap(buf);

						// おｋ
						return true;
					}
				}
			}
			catch
			{
				// プラグインエラー
				MessageBox.Show("プラグインで内部エラーが発生しました。", "プラグインエラー", MessageBoxButtons.OK, MessageBoxIcon.Warning);

				// だめだったのよ～
				return false;
			}

			// 正しく対応できなかった。
			return false;
		}

		#region Don't use Method's

		public byte[] GetFrameAudio(int index)
		{
			throw new Exception("The method or operation is not implemented.");
		}

		public byte[] GetAudio()
		{
			throw new Exception("The method or operation is not implemented.");
		}

		#endregion

		#region IDisposable メンバ

		public void Dispose()
		{
			// throw new Exception("The method or operation is not implemented.");
		}

		#endregion
	}
}
