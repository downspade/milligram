using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Text;

namespace BxSpi
{
    /// <summary>画像展開用クラス</summary>
    public sealed class SpiPicture : SpiCore
    {
        /// <summary>画像情報を取得するデリゲートらしい</summary>
        private SpiGetPictureInfo getPictureInfo;
        /// <summary>画像を取得するデリゲートらしい</summary>
        private SpiGetPicture getPicture;

        /// <summary>*.spiファイルを読み込みます。</summary>
        /// <param name="fileName">読み込む*.spi</param>
        public SpiPicture(string fileName)
            : base(fileName)
        {
            getPictureInfo = (SpiGetPictureInfo)Win32.GetProcAddress(typeof(SpiGetPictureInfo), handle, "GetPictureInfo");
            getPicture = (SpiGetPicture)Win32.GetProcAddress(typeof(SpiGetPicture), handle, "GetPicture");
        }

        /// <summary>指定したファイルの画像情報を取得します。</summary>
        /// <param name="fileName">取得するファイル</param>
        /// <returns>取得できたか</returns>
        public bool GetPictureInfo(string fileName, out PictureInfo pictureInfo)
        {
            // ファイルが対応しているかチェック
            if (getPictureInfo(fileName, 0, 0, out pictureInfo) == 0)
                return true;

            // だめだったら、Falseを返す
            return false;
        }

        /// <summary>指定したファイルからビットマップを作成します。</summary>
        /// <param name="fileName">読み込むファイル</param>
        /// <returns>ビットマップ</returns>
        public Bitmap GetPicture(string fileName)
        {
            PictureInfo info;

            // インフォメーションを呼び出す。
            if (GetPictureInfo(fileName, out info))
            {
                // BITMAPINFO,BITMAPデータ
                IntPtr bmpInfo, bmpData;

                // 画像を取得します。
                if (getPicture(fileName, 0, 0, out bmpInfo, out bmpData, IntPtr.Zero, 0) == 0)
                {
                    try
                    {
                        // 構造体をもってけｗ
                        Win32.BITMAPINFO bpi = (Win32.BITMAPINFO)Marshal.PtrToStructure(Marshal.ReadIntPtr(bmpInfo), typeof(Win32.BITMAPINFO));

                        // ビットマップ
                        Bitmap bmp;
                        // データサイズ
                        int dataSize;

                        // カラービットの検査
                        if (info.colorDepth == 32)
                        {
                            bmp = new Bitmap(info.width, info.height, PixelFormat.Format32bppArgb);
                            dataSize = info.width * info.height * 4;
                        }
                        else if (info.colorDepth == 24)
                        {
                            bmp = new Bitmap(info.width, info.height, PixelFormat.Format24bppRgb);
                            dataSize = info.width * info.height * 3;
                        }
                        else if (info.colorDepth == 16)
                        {
                            bmp = new Bitmap(info.width, info.height, PixelFormat.Format16bppRgb565);
                            dataSize = info.width * info.height * 3;
                        }
                        else if (info.colorDepth == 8)
                        {
                            bmp = new Bitmap(info.width, info.height, PixelFormat.Format8bppIndexed);
                            dataSize = info.width * info.height;
                        }
                        else if (info.colorDepth == 4)
                        {
                            bmp = new Bitmap(info.width, info.height, PixelFormat.Format4bppIndexed);
                            dataSize = info.width * info.height / 2;
                        }
                        else
                        {
                            bmp = new Bitmap(info.width, info.height, PixelFormat.Format1bppIndexed);
                            dataSize = info.width * info.height / 8;
                        }

                        // カラーパレット
                        ColorPalette cp = bmp.Palette;

                        for (int i = 0; i < cp.Entries.Length; i++)
                            cp.Entries[i] = Color.FromArgb(bpi.bmiColors[i].rgbRed, bpi.bmiColors[i].rgbGreen, bpi.bmiColors[i].rgbBlue);

                        bmp.Palette = cp;

                        // メモリにロックする
                        BitmapData bitmapdata = bmp.LockBits(new Rectangle(new Point(), bmp.Size), ImageLockMode.ReadWrite, bmp.PixelFormat);

                        // RGBデータ保持用の配列
                        byte[] data = new byte[dataSize <= 1 ? 1 : dataSize];

                        // RGBデータ配列にRGBデータをコピー
                        Marshal.Copy(Marshal.ReadIntPtr(bmpData), data, 0, data.Length);

                        // 配列のデータを転送(Bitmap本体)
                        Marshal.Copy(data, 0, bitmapdata.Scan0, data.Length);

                        // メモリから解放してあげる
                        bmp.UnlockBits(bitmapdata);

                        // 出力されたイメージは上下逆なので反転
                        bmp.RotateFlip(RotateFlipType.RotateNoneFlipY);

                        // イメージを出力
                        return bmp;
                    }
                    finally
                    {
                        // 使用しないメモリを除去
                        if (info.hInfo != null)
                            Marshal.FreeHGlobal(info.hInfo);

                        // 使用しないメモリを除去
                        if (bmpData != null)
                            Marshal.FreeHGlobal(bmpData);

                        // 使用しないメモリを除去
                        if (bmpInfo != null)
                            Marshal.FreeHGlobal(bmpInfo);
                    }
                }
                // だめだったら無を返す。
                return null;
            }

            // だめだったら無を返す。
            return null;
        }

        /// <summary>Spiの入出力情報を取得します</summary>
        public override ApiVersionInfomation GetApiInfo()
        {
            return base.GetApiInfo();
        }

        /// <summary>プラグイン情報を取得します。</summary>
        public override string GetApiVersion()
        {
            return base.GetApiVersion();
        }

        /// <summary>拡張子を取得します。</summary>
        public override string GetCorrespondingType()
        {
            return base.GetCorrespondingType();
        }

        /// <summary>拡張子の説明を返します。</summary>
        public override string GetCorrespondingName()
        {
            return base.GetCorrespondingName();
        }

        /// <summary>Disposeが呼ばれたら破棄します。</summary>
        /// <param name="disposing">破棄フラグ</param>
        protected override void Dispose(bool disposing)
        {
            if (!base.disposed)
            {
                if (disposing)
                {
                    // デリゲートを破棄します。
                    getPictureInfo = null;
                    getPicture = null;
                }
                base.Dispose(disposing);
            }
        }
    }
}
