using System;
using System.Drawing;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using Factory;
using System.Windows.Forms;
using System.Drawing.Imaging;
using System.Threading;

namespace milligram
{
    public enum EPluginMode : int
    {
        NONE = 0,
        DNET = 1,
        SPI = 2,
        PICTURE = 3,
        ARCHIVE = 16,
        ACVINNER = 32,
        CLEARFILE = 128,
        ALL = 255,
    }

    public enum EWorkFileType : int
    {
        NONE = 0,
        DNET_PICTURE = 1,
        DNET_ARCHIVE = 2,
        SPI_PICTURE = 3,
        SPI_ARCHIVE = 4,
    }


    public enum ENoSPIPictureType : int
    {
        NONE = 0,
        BMP = 1,
        JPG = 2,
        PNG = 3,
        GIF = 4,
        TIF = 5,
    }

    //---------------------------------------------------------------------------

    // ============================================================================
    //			DLL 外部関数を呼ぶためのデリゲート
    // ============================================================================

    // プラグインの情報を得る
    public delegate int SpiGetPluginInfo(int infono, IntPtr buf, int buflen);
    //  展開可能なファイル形式か調べる
    public delegate int SpiIsSupported([MarshalAs(UnmanagedType.LPStr)]string filename, IntPtr dw);
    //  画像ファイルに関する情報を得る　
    public delegate int SpiGetPictureInfo(IntPtr buf, int len, uint flag, out PictureInfo lpInfo);
    //  画像を展開する
    public delegate int SpiGetPicture(IntPtr buf, int len, uint flag, out IntPtr pHBInfo, out IntPtr pHBm, CallbackDelegate lpPrgressCallback, int lData);
    //  プレビュー・カタログ表示用画像縮小展開ルーティン *
    public delegate int SpiGetPreview(IntPtr buf, int len, uint flag, out IntPtr pHBInfo, out IntPtr pHBm, CallbackDelegate lpPrgressCallback, int lData);
    //  アーカイブ内のすべてのファイルの情報を取得する
    public delegate int SpiGetArchiveInfo(IntPtr buf, int len, uint flag, out IntPtr lphInf);
    //  アーカイブ内のすべてのファイルの情報を取得する
    public delegate int SpiGetFileInfo(IntPtr buf, int len, uint flag, out IntPtr lphInf);
    //  アーカイブ内のファイルを取得する
    public delegate int SpiGetFile(IntPtr buf, int len, out IntPtr dest, uint flag, CallbackDelegate lpPrgressCallback, int lData);
    // Plug-in設定ダイアログの表示
    public delegate int SpiConfigurationDlg(IntPtr parent, int fnc);


    // コールバック Delegate の定義

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate int CallbackDelegate(int nNum, int nDenom, uint lData);

    // ============================================================================
    //			SUSIE plug-in から情報を取得するための構造体
    // ============================================================================

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct PictureInfo
    {
        public int Left;     // 展開する位置
        public int Top;
        public uint Width;     // 画像の幅
        public uint Height;        //	   高さ
        public ushort X_Density;     // 画素の水平方向密度
        public ushort Y_Density;     // 画素の垂直方向密度
        public short Color;            // １画素のbit数
        public IntPtr Info;            // 画像のテキストヘッダ
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    unsafe public struct ArchivedFileInfo
    {
        public fixed byte Method[8];   // 圧縮法の種類
        public uint Position;    // ファイル上での位置
        public uint CompSize;    // 圧縮されたサイズ
        public uint FileSize;    // 元のファイルサイズ
        public int Timestamp;          // ファイルの更新日時
        public fixed byte Path[200];            // 相対パス
        public fixed byte FileName[200];        // ファイルネーム
        public uint CRC;         //CRC
    };


    // 画像の情報と表示方法を保存するための構造体
    public struct ImageInfo
    {
        public string FileName;
        public DateTimeOffset Timestamp;
        public uint FileSize;
        public int Rotate;
        public List<ImageInfo> imageInfoList;
    };

    // ============================================================================
    //			SUSIE plug-in プロシージャ管理テーブル
    // ============================================================================
    public class SpiPlugin : IDisposable
    {
        protected IntPtr IntPtr = IntPtr.Zero; // Spiのハンドルを保持するポインタ
        protected bool disposed = false; // 破棄フラグ

        public SpiGetPluginInfo GetPluginInfo; 
        public SpiIsSupported IsSupported;
        public SpiGetPictureInfo GetPictureInfo;
        public SpiGetPicture GetPicture;
        public SpiGetPreview GetPreview;
        public SpiGetArchiveInfo GetArchiveInfo;
        public SpiGetFileInfo GetFileInfo;
        public SpiGetFile GetFile;
        public SpiConfigurationDlg ConfigurationDlg; 

        public string APIVersion;
        public string PluginName;

        public bool LoadSpiLL(string fileName) // *.spiファイルを読み込み
        {
            // 指定したDLLファイルを呼び出します。
            IntPtr = Win32.LoadLibrary(fileName);

            // デリゲートを初期化？する
            GetPluginInfo = (SpiGetPluginInfo)Win32.GetProcAddressDelegate(typeof(SpiGetPluginInfo), IntPtr, "GetPluginInfo");
            IsSupported = (SpiIsSupported)Win32.GetProcAddressDelegate(typeof(SpiIsSupported), IntPtr, "IsSupported");
            GetPictureInfo = (SpiGetPictureInfo)Win32.GetProcAddressDelegate(typeof(SpiGetPictureInfo), IntPtr, "GetPictureInfo");
            GetPicture = (SpiGetPicture)Win32.GetProcAddressDelegate(typeof(SpiGetPicture), IntPtr, "GetPicture");
            GetPreview = (SpiGetPreview)Win32.GetProcAddressDelegate(typeof(SpiGetPreview), IntPtr, "GetPreview");
            GetArchiveInfo = (SpiGetArchiveInfo)Win32.GetProcAddressDelegate(typeof(SpiGetArchiveInfo), IntPtr, "GetArchiveInfo");
            GetFileInfo = (SpiGetFileInfo)Win32.GetProcAddressDelegate(typeof(SpiGetFileInfo), IntPtr, "GetFileInfo");
            GetFile = (SpiGetFile)Win32.GetProcAddressDelegate(typeof(SpiGetFile), IntPtr, "GetFile");
            ConfigurationDlg = (SpiConfigurationDlg)Win32.GetProcAddressDelegate(typeof(SpiConfigurationDlg), IntPtr, "ConfigurationDlg");

            if (GetPluginInfo == default) return (false);

            IntPtr ptr = Marshal.AllocHGlobal(5);
            GetPluginInfo(0, ptr, 5);
            APIVersion = Marshal.PtrToStringAnsi(ptr);

            ptr = Marshal.AllocHGlobal(200);
            GetPluginInfo(1, ptr, 200);
            PluginName = Marshal.PtrToStringAnsi(ptr);

            return (true);
        }

        // Disposeが呼ばれたら破棄します。 
        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (disposing)
                {
                    // デリゲートを破棄します。
                    GetPluginInfo = null;
                    IsSupported = null;
                    GetPicture = null;
                    GetPreview = null;
                    GetArchiveInfo = null;
                    GetFileInfo = null;
                    GetFile = null;
                    ConfigurationDlg = null;

                    // 使用したDLLを破棄します。
                    // if (!Win32.FreeLibrary(IntPtr))
                    //    throw new Exception("Spiプラグインの破棄ができませんでした。\nハンドル : " + IntPtr.ToString());
                }
                disposed = true;
            }
        }

        // Disposeメソッドです。 
        public void Dispose()
        {
            // 使用したDLLを破棄します。
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        // デストラクタ 
        ~SpiPlugin()
        {
            // デストラクタによる破棄は行いません。
            Dispose(false);
        }
    }

    // ============================================================================
    //			SUSIE plug-in 画像取得管理クラス
    // ============================================================================
    public class SpiLoader
    {
        public bool ErrorFlag;             // エラー時表示フラグ
        public ImageInfo ImageInfo;          // File name
        public ImageInfo SubImageInfo; // Archive 中のFile Name
        public ImageInfo PostImageInfo;  // 読込中の File name
        public string PostFileExt; // Susie に渡す用の FileExt

        //
        public List<SpiPlugin> Spi;               // プラグインリスト
        public SpiPlugin NowSpi;         // 最後に確定したプラグイン
        public SpiPlugin PostSpi;    // 次に読むこ無事が確定した Spi プラグイン
        public SpiPlugin ArchiveSpi; // 現在解凍に使っている Spi プラグイン

        public IntPtr hBitmap; // 転送用の hBitmap
        public Win32.BITMAPINFO pBmpInfo; // 転送用のビットマップインフォ
        public IntPtr pBmpData; // 転送用の ビットマップデータ先頭アドレス
        public IntPtr hBmpData; // 転送用のハンドル


        // ハンドルデータ
        public IntPtr org_HBmpData, org_HBmpInfo; // SusiePlugin がもってくる現在表示中のハンドル
        public IntPtr HFileImage, org_HFileImage; // Susie プラグインが持ってくるハンドル
        public IntPtr pFileImage, org_pFileImage; // データへのポインタ
        public IntPtr org_pBmpData; // 読み込んだ画像のオリジナルデータ
        public Win32.BITMAPINFO org_pBmpInfo; // 読み込んだ画像のオリジナルデータ


        public List<ArchivedFileInfo> nowArchivedFileInfo;
        public List<ArchivedFileInfo> oldArchivedFileInfo;

        public bool MustTrans;

        //
        public EWorkFileType eWorkFileType;

        // ポインタデータ


        public Bitmap BitmapDNET;
        public Image ImageDNET;
        private MemoryStream memoryStream;

        // GIF アニメーション関連
        public bool GIFAnimate;
        public int LoopCount;
        private int LoopIndex;
        public int FrameCount;
        private int FrameIndex;
        private int[] Delay;
        public int DelayTime;
        public int DropFrame;
        private int DropCount;

        public bool Showing; // 表示されているかどうか

        private IntPtr pbgBmpData; // データへのポインタ
        private Win32.BITMAPINFO pbgBmpInfo;
        private IntPtr hbgBmpData; // バックバッファーのBitmapハンドル


        public int Err;                    // Win32 エラー番号
        public int OrgWidth, OrgHeight; // もと画像の大きさ
        public int ORotWidth, ORotHeight; // 転送もと画像のかたち(回転後)

        private int BufWidth, BufHeight; // 転送もとのフォームの大きさ（回転前）
        public int SrcRWidth, SrcRHeight;   // 転送もとの大きさ(回転後)
        private int BufMaxWidth, BufMaxHeight; // バックバッファーの画像の大きさ
        public EPluginMode Mode;

        public int MaxBBufferPixel = 4000 * 4000; // バックバッファーの最大ピクセルサイズ 4000 x 4000 は 64M

        public CallbackDelegate ProgressCallback;

        public int ImageNum;
        public int Rotate; // 現在の回転値
        public bool FixRotate; // 回転を固定するかどうか
        public bool NoRotationRefresh;

        ENoSPIPictureType NOSPIType;
        ENoSPIPictureType PostNOSPIType;

        public string InternalLoader;

        public IntPtr FormHandle;
        object syncObject = new object();

        public string PluginName;
        private uint ShortestPeriod; // 計算できる最小間隔
        private uint NowTGT;
        private uint PreTGT;
        private uint PausedTGT;

        //---------------------------------------------------------------------------

        public SpiLoader(CallbackDelegate aProgressCallback)
        {
            ProgressCallback = aProgressCallback;
            ErrorFlag = true;
            InternalLoader = "*.bmp;*.jpg;*.jpeg;*.png;*.gif;";
            Spi = new List<SpiPlugin>();

            ImageInfo = new ImageInfo();          // File name
            SubImageInfo = new ImageInfo(); // Archive 中のFile Name
            PostImageInfo = new ImageInfo();  // 読込中の File name

            // マルチメディアタイマーの設定
            unsafe
            {
                Win32.TIMECAPS TimeCaps = default;
                Win32.timeGetDevCaps(ref TimeCaps, (uint)sizeof(Win32.TIMECAPS));
                ShortestPeriod = TimeCaps.wPeriodMin;
            }

            Win32.timeBeginPeriod(ShortestPeriod);

        }

        /*        ~TSpiLoader() デストラクタはなし
                {
                    Clear(MMODE_ALL);
                    DeleteDBuffer();

                    if (Spi != null)
                    {
                        int i;
                        for (i = 0; i < SpiNum; i++)
                        {
                            delete Spi[i];
                        }
                        delete[] Spi;
                    }
                    Mode = 0;
                }*/

        public void Dispose()
        {
            Win32.EndPeriod(ShortestPeriod);
            Clear(EPluginMode.ALL);
            CheckBackBuffer(0, 0);
        }

        // プラグイン一覧を取得し、実際に、プラグインを読み込む
        public void SetSpiPathes(ref List<string> Pathes)
        {
            int i;
            SpiPlugin Plg;

            List<string> SpiFullLists = new List<string>(); // フルパス
            List<string> SpiNameLists = new List<string>(); // ファイル名のみ
            List<string> SpiGetLists = new List<string>(); // 最終的に取得すべきフルパス

            // 古いポインタを削除する
            if (Spi.Count > 0)
            {
                foreach (SpiPlugin a in Spi)
                {
                    a.Dispose();
                }
                Spi.Clear();
            }

            //
            SpiFullLists = new List<string>();
            SpiNameLists = new List<string>();
            SpiGetLists = new List<string>();

            // 全部のパスに入っている spi ファイルを取得
            for (i = 0; i < Pathes.Count; i++)
            {
                string[] files = Directory.GetFiles(Pathes[i], "*.spi");
                foreach (string a in files) SpiFullLists.Add(a);
            }

            // ファイル名だけで spi ファイルを取得してコピーしておく
            foreach (string a in SpiFullLists)
                SpiNameLists.Add(Path.GetFileName(a));

            // 同じファイル名の spi ファイルは１回しか読まないようにしておく
            for (i = 0; i < SpiFullLists.Count; i++)
            {
                if (SpiGetLists.IndexOf(SpiNameLists[i]) < 0)
                    SpiGetLists.Add(SpiFullLists[i]);
            }

            // 一つずつ、LoadLibrary してみる
            for (i = 0; i < SpiGetLists.Count; i++)
            {
                Plg = new SpiPlugin();
                if
                (
                    Plg.LoadSpiLL(SpiGetLists[i]) == true
                    && (Plg.APIVersion == "00IN"
                        || Plg.APIVersion == "00AM")
                )
                    Spi.Add(Plg);
            }

            Showing = false;
        }

        // リフレッシュ
        public bool Refresh()
        {
            ImageInfo OldFileInfo = ImageInfo;
            ImageInfo OldSubFileInfo = SubImageInfo;

            Clear(EPluginMode.ALL);
            bool Result;

            if (OldSubFileInfo.FileName == "")
            {
                Result = SetImageFile(ref OldFileInfo);
            }
            else
            {
                Result = SetImageFile(ref OldFileInfo) & SetSubImageFile(ref OldSubFileInfo);
            }
            return (Result);
        }

        // 読み込むファイルを設定する
        public bool SetImageFile(ref ImageInfo SrcImageInfo)
        {
            long Size;
            bool Result = false;

            IntPtr hFile;
            IntPtr hMap;

            // ファイルハンドルの取得
            hFile = Win32.CreateFileW(SrcImageInfo.FileName, Win32.EFileAccess.GENERIC_READ /*| Win32.EFileAccess.GENERIC_WRITE*/,
                    Win32.EFileShare.FILE_SHARE_WRITE, default, Win32.ECreationDisposition.OPEN_EXISTING,
                    Win32.EFileAttributes.FILE_ATTRIBUTE_NORMAL, default);

            if (hFile != default && hFile != Win32.INVALID_HANDLE_VALUE)
            {

                IntPtr FileData;

                // ファイルはすでに存在していて、この関数は成功します
                Result = Win32.GetFileSizeEx(hFile, out Size);

                if (Result)
                {
                    // メモリマップドファイルを作成する

                    hMap = Win32.CreateFileMapping(hFile, IntPtr.Zero, Win32.EFileMapProtection.PAGE_READONLY, 0, 0, default);
                    if (hMap != default)
                    {
                        FileData = Win32.MapViewOfFile(hMap, Win32.EFileMapAccessType.FILE_MAP_READ, 0, 0, 0);
                        PostSpi = null;

                        Result = CheckFileType(ref SrcImageInfo, FileData, Size);

                        if (Result)
                        {
                            // 実際にマップドされたメモリから読み込む
                            Result = LoadFromFileInMemory(ref SrcImageInfo, FileData, (uint)Size, ProgressCallback);
                        }

                        // メモリマップドファイルの終了 
                        Win32.UnmapViewOfFile(FileData);
                    }
                    Win32.CloseHandle(hMap);
                }
            }
            Win32.CloseHandle(hFile);

            return (Result);
        }

        public void InitSize(int iWidth, int iHeight)
        {
            OrgWidth = iWidth;
            OrgHeight = iHeight; // もと画像の大きさ
            ORotWidth = iWidth;
            ORotHeight = iHeight; // 転送もと画像のかたち(回転後)

            SrcRWidth = iWidth;
            SrcRHeight = iHeight;   // 転送もとの大きさ(回転後)
        }

        private bool CheckFileType(ref ImageInfo SrcImageInfo, IntPtr FileData, long FileSize)
        {
            bool Result = false;
            eWorkFileType = EWorkFileType.NONE;
            if (FileData == default) return (false);

            Result = CheckGifAnime(ref SrcImageInfo, FileData, FileSize);
                
            if (Acfc.FileMasks(SrcImageInfo.FileName, InternalLoader))
            {
                if (Result == false) Result = CheckFileTypeDNET(ref SrcImageInfo, FileData);
                if (Result == false) Result = CheckFileTypeSPI(ref SrcImageInfo, FileData);
            }
            else
            {
                if (Result == false) Result = CheckFileTypeSPI(ref SrcImageInfo, FileData);
                if (Result == false) Result = CheckFileTypeDNET(ref SrcImageInfo, FileData);
            }
            return (Result);
        }

        private bool CheckFileTypeSPI(ref ImageInfo SrcImageInfo, IntPtr FileData)
        {
            // -----------------------------------------------------------------------------
            // まず SPI プラグインで読み込めるかどうか調べる
            try
            {
                PostFileExt = Path.GetExtension(SrcImageInfo.FileName);
                int i;
                // SPI プラグインが必要かどうか調べる。
                for (i = 0; i < Spi.Count; i++)
                {
                    if (Spi[i].IsSupported(PostFileExt, FileData) != 0)
                    {
                        PostSpi = Spi[i];
                        PostImageInfo = SrcImageInfo;

                        if (PostSpi.APIVersion == "00IN")
                            eWorkFileType = EWorkFileType.SPI_PICTURE;
                        else if(PostSpi.APIVersion == "00AM")
                            eWorkFileType = EWorkFileType.SPI_ARCHIVE;

                        return (true);
                    }
                }
            }
            catch
            {
                return (false);
            }

            // -----------------------------------------------------------------------------
            return (false);
        }

        private unsafe bool CheckGifAnime(ref ImageInfo SrcImageInfo, IntPtr FileData, long FileSize)
        {
            GIFAnimate = false;
            if (FileSize < 18) return (true);
            int i, t, AnimFlag = 0;

            byte* ByteData = (byte*)FileData.ToPointer();

            byte[] GIF8s = { 0x47, 0x49, 0x46, 0x38 }; // GIF8
            fixed (byte* GIF8 = GIF8s)
            {
                if (Acfc.MemNCmp(ByteData, GIF8, 4) != 0) return (false);
            }
            if (*((byte*)(ByteData + 4)) != '9' && *((byte*)(ByteData + 4)) != '7') return (false);
            if (*((byte*)(ByteData + 5)) != 'a') return (false);

            // ヘッダのチェック --------------------
            // gif かどうか調べる

            if ((ByteData[10] & 0x80) != 0)// Global Color Table が存在する
            {
                t = (1 << ((ByteData[10] & 0x07) + 1)) * 3;
                i = t;
            }
            else
            {
                i = 0;
            }
            i += 13;
            // --------------------

            while (true)
            {
                if (ByteData[i] == 0x2C)
                {
                    // Image Block
                    i += 9; if (i >= FileSize) break;

                    if ((ByteData[i] & 0x80) > 0) // Local Color Table が存在する
                    {
                        t = (1 << ((ByteData[i] & 0x07) + 1)) * 3;
                        i += t;
                    }

                    i += 2; if (i >= FileSize) break;

                    do
                    {
                        i += ByteData[i] + 1;
                        if (i >= FileSize) break;
                    }
                    while (ByteData[i] != 0);

                    if (AnimFlag == 1) AnimFlag = 2;
                }
                else if (ByteData[i] == 0x21)
                {
                    i++; if (i >= FileSize) break;

                    if (ByteData[i] == 0xf9)
                    {
                        // Graphic Control Extension
                        i++; if (i >= FileSize) break;

                        if (ByteData[i] != 0x04) break;

                        i += 5; if (i >= FileSize) break;

                        if (ByteData[i] != 0x00) break;

                        if (AnimFlag == 0) AnimFlag = 1;
                    }
                    else if (ByteData[i] == 0xfe || ByteData[i] == 0x01 || ByteData[i] == 0xff)
                    {
                        // Comment Extension, Plain Text Extension & Application Extension
                        i++; if (i >= FileSize) break;

                        do
                        {
                            i += ByteData[i] + 1;
                            if (i >= FileSize) break;
                        }
                        while (ByteData[i] != 0);
                    }
                }
                else return (false);

                i++; if (i >= FileSize) break;

                if (ByteData[i] == 0x3b) break;
            }

            if (AnimFlag != 2) return (false);

            PostSpi = null;
            PostImageInfo = SrcImageInfo;
            PostNOSPIType = ENoSPIPictureType.GIF;
            eWorkFileType = EWorkFileType.DNET_PICTURE;
            GIFAnimate = true;
            return (true);
        }

        private unsafe bool CheckFileTypeDNET(ref ImageInfo SrcImageInfo, IntPtr FileData)
        {
            byte* ByteData = (byte*)FileData;
            byte[] BMs = { 0x42, 0x4D }; // BitMap
            byte[] PNGs = { 0x50, 0x4E, 0x47 }; // PNG
            byte[] GIFs = { 0x47, 0x49, 0x46 }; // GIF
            byte[] TIFFls = { 0x49, 0x49, 0x2A, 0x00 }; // TIFF LITTLE Endian
            byte[] TIFFbs = { 0x4D, 0x4D, 0x00, 0x2A }; // TIFF BIG Endian

            // 通常のビットマップかどうか調べる
            fixed (byte* BM = BMs)
            {
                if (Acfc.MemNCmp(ByteData, BM, BMs.Length) == 0)
                {
                    PostSpi = null;
                    PostImageInfo = SrcImageInfo;
                    PostNOSPIType = ENoSPIPictureType.BMP;
                    eWorkFileType = EWorkFileType.DNET_PICTURE;
                    return (true);
                }
            }

            // jpeg かどうか調べる
            if ((int)((byte*)ByteData[0]) == 0xFF && (int)((byte*)ByteData[1]) == 0xD8)
            {
                PostSpi = null;
                PostImageInfo = SrcImageInfo;
                PostNOSPIType = ENoSPIPictureType.JPG;
                eWorkFileType = EWorkFileType.DNET_PICTURE;
                return (true);
            }

            // PNG かどうか調べる
            fixed (byte* PNG = PNGs)
            {
                if (Acfc.MemNCmp(ByteData, PNG, PNGs.Length) == 0)
                {
                    PostSpi = null;
                    PostImageInfo = SrcImageInfo;
                    PostNOSPIType = ENoSPIPictureType.PNG;
                    eWorkFileType = EWorkFileType.DNET_PICTURE;
                    return (true);
                }
            }

            // gif かどうか調べる
            fixed (byte* GIF = GIFs)
            {
                if (Acfc.MemNCmp(ByteData, GIF, GIFs.Length) == 0)
                {
                    PostSpi = null;
                    PostImageInfo = SrcImageInfo;
                    PostNOSPIType = ENoSPIPictureType.GIF;
                    eWorkFileType = EWorkFileType.DNET_PICTURE;
                    return (true);
                }
            }

            // tif かどうか調べる
            fixed (byte* TIF = TIFFls)
            {
                if (Acfc.MemNCmp(ByteData, TIFFls, TIFFls.Length) == 0 || Acfc.MemNCmp(ByteData, TIFFbs, TIFFbs.Length) == 0)
                {
                    PostSpi = null;
                    PostImageInfo = SrcImageInfo;
                    PostNOSPIType = ENoSPIPictureType.TIF;
                    eWorkFileType = EWorkFileType.DNET_PICTURE;
                    return (true);
                }
            }
            return (false);
        }

        // 画像ファイルを実際に読み込む
        private unsafe bool LoadFromFileInMemory(ref ImageInfo SrcImageInfo, IntPtr FileData, uint DataSize, CallbackDelegate ProgressCallback)
        {
            if (FileData == default) return (false);
            PictureInfo PInfo;
            byte* ByteData = (byte*)FileData;

            Showing = false;

            // アーカイブモードかどうかチェックする
            // アーカイブモードならファイルの一覧を取得する
            if (PostSpi != default && PostSpi.APIVersion == "00AM" && (SubImageInfo.FileName == default || SubImageInfo.FileName == ""))
            {
                IntPtr HInfo;
                if (PostSpi.GetArchiveInfo(FileData, (int)DataSize, 1, out HInfo) != 0) return (false);

                IntPtr pInfos = Win32.LocalLock(HInfo);
                IntPtr pInfos_b = pInfos;
                nowArchivedFileInfo = new List<ArchivedFileInfo>();

                ArchivedFileInfo pInfo = *(ArchivedFileInfo*)pInfos;
                while (pInfo.Method[0] != '\0')
                {
                    nowArchivedFileInfo.Add(pInfo);
                    pInfos += sizeof(ArchivedFileInfo);
                    pInfo = *(ArchivedFileInfo*)pInfos;
                    ImageNum++;
                }

                Win32.LocalFree(pInfos_b);

                Clear(EPluginMode.PICTURE | EPluginMode.ARCHIVE);

                oldArchivedFileInfo = nowArchivedFileInfo;

                Mode = EPluginMode.ARCHIVE;

                ArchiveSpi = PostSpi;
                ImageInfo = PostImageInfo;

                return (true);
            }

            // SPI モードでない場合、素でファイルを読み込んでみる
            if (PostSpi == default)
            {
                Clear(EPluginMode.PICTURE);

                // GIFAnimate の場合にはメモリストリームを保存する必要がある
                if (GIFAnimate)
                {
                    memoryStream = new MemoryStream();
                    UnmanagedMemoryStream ums = new UnmanagedMemoryStream((byte*)FileData, DataSize);
                    ums.CopyTo(memoryStream);
                    try
                    {
                        ImageDNET = Image.FromStream(memoryStream, false, false);
                    }
                    catch
                    {
                        GIFAnimate = false;
                        return (false);
                    }
                }
                else
                {
                    UnmanagedMemoryStream ums = new UnmanagedMemoryStream((byte*)FileData, DataSize);
                    try
                    {
                        ImageDNET = Image.FromStream(ums, false, false);
                    }
                    catch
                    {
                        return (false);
                    }
                }

                OrgWidth = ImageDNET.Width;
                OrgHeight = ImageDNET.Height;

                BitmapDNET = new Bitmap(ImageDNET);
                hBmpData = hBitmap = BitmapDNET.GetHbitmap();

                Mode &= (EPluginMode.ALL ^ EPluginMode.PICTURE);
                Mode |= EPluginMode.DNET;

                if (FixRotate == false)
                {
                    Rotate = SrcImageInfo.Rotate;
                    if(Rotate < 0) Rotate = CheckOrientation();
                }
                else
                {
                    if (Rotate < 0) Rotate = CheckOrientation();
                }
                SrcImageInfo.Rotate = Rotate;

                if (GIFAnimate == true)
                    GIFAnimate = CheckGIFAnimData();

                MustTrans = true;
                PluginName = "Internal Loader";
            }
            else if (PostSpi.APIVersion == "00IN")
            {
                if (FixRotate == false)
                {
                    Rotate = SrcImageInfo.Rotate;
                    if(Rotate < 0) Rotate = CheckOrientation(ByteData, (int)DataSize);
                }
                else
                {
                    if (Rotate < 0) Rotate = CheckOrientation(ByteData, (int)DataSize);
                }
                SrcImageInfo.Rotate = Rotate;

                if (PostSpi.GetPictureInfo(FileData, (int)DataSize, 1, out PInfo) != 0) return (false);
                Win32.GlobalFree(PInfo.Info);         // TEXT info 解放

                IntPtr HBmpInfo, HBmpData;

                if (PostSpi.GetPicture(FileData, (int)DataSize, 1, out HBmpInfo, out HBmpData, ProgressCallback, 0) != 0) return (false);

                Clear(EPluginMode.PICTURE);

                pBmpData = Win32.GlobalLock(HBmpData); // Win32 API でロック
                pBmpInfo = *(Win32.BITMAPINFO*)Win32.GlobalLock(HBmpInfo); 

                OrgWidth = pBmpInfo.bmiHeader.biWidth;
                OrgHeight = pBmpInfo.bmiHeader.biHeight;

                org_HBmpData = HBmpData;
                org_HBmpInfo = HBmpInfo;
                org_pBmpData = pBmpData;
                org_pBmpInfo = pBmpInfo;

                IntPtr hdc = Win32.GetWindowDC(default);

                hBmpData = hBitmap = Win32.CreateDIBitmap(hdc, ref pBmpInfo.bmiHeader, Win32.EDIBInitializeMode.CBM_INIT, pBmpData, ref pBmpInfo, Win32.EDIBColorMode.DIB_RGB_COLORS);

                Win32.DeleteObject(hdc);

                Mode &= (EPluginMode.ALL ^ EPluginMode.PICTURE);
                Mode |= EPluginMode.SPI;

                PluginName = PostSpi.PluginName;
            }
            else return (false);

            BufWidth = ORotWidth = SrcRWidth = OrgWidth;
            BufHeight = ORotHeight = SrcRHeight = OrgHeight;

            if(Rotate % 2 == 0)
            {
                ORotWidth = OrgWidth;
                ORotHeight = OrgHeight;
            }
            else
            {
                ORotWidth = OrgHeight;
                ORotHeight = OrgWidth;
            }

            NowSpi = PostSpi;
            NOSPIType = PostNOSPIType;

            Showing = true;

            if (Rotate != 0) MustTrans = true;

            return (true);
        }

        public bool ChangeArchiveFileName(string NewFileName)
        {
            ImageInfo.FileName = NewFileName;
            return (true);
        }

        public int LoadSubIndex(ref int SubIndex, int Ofs)
        {
            if ((Mode & EPluginMode.ARCHIVE) == 0) return (0);
            while (SubIndex < 0) SubIndex += ImageNum;
            while (SubIndex >= ImageNum) SubIndex -= ImageNum;

            int OldSubIndex = SubIndex;
            while (true)
            {
                if (SetShowSubIndex(ref SubImageInfo, SubIndex, ProgressCallback) == true) return (1);

                SubIndex += Ofs;
                while (SubIndex < 0) SubIndex += ImageNum;
                while (SubIndex >= ImageNum) SubIndex -= ImageNum;
                if (SubIndex == OldSubIndex)
                {
                    SubIndex = -1;
                    break;
                }
            }
            return (0);
        }

        /// <summary>
        /// バックバッファのサイズを調べて小さければ大きくする
        /// sWidth か sHeight に 0 を入れると開放だけ行う
        /// </summary>
        /// <param name="sWidth"></param>
        /// <param name="sHeight"></param>
        private unsafe void CheckBackBuffer(int sWidth, int sHeight)
        {
            if(sWidth > 0 && sHeight > 0 && BufMaxWidth >= sWidth && BufMaxHeight >= sHeight) return;

            int cWidth;
            int cHeight;
            if(sWidth > sHeight)
            {
                cWidth = sWidth;
                cHeight = sWidth;
            }
            else
            {
                cWidth = sHeight;
                cHeight = sHeight;
            }

            if(cWidth * cHeight > MaxBBufferPixel)
            {
                cWidth = sWidth;
                cHeight = sHeight;
            }

            if (hbgBmpData != default)
            {
                Win32.DeleteObject(hbgBmpData);
            }

            if (sWidth == 0 || sHeight == 0) return;

            pbgBmpInfo = new Win32.BITMAPINFO();
            pbgBmpInfo.bmiHeader.biSize = (uint)sizeof(Win32.BITMAPINFOHEADER);

            pbgBmpInfo.bmiHeader.biPlanes = 1;
            pbgBmpInfo.bmiHeader.biBitCount = 32;
            pbgBmpInfo.bmiHeader.biCompression = Win32.EBitmapCompressionMode.BI_RGB;
            pbgBmpInfo.bmiHeader.biWidth = cWidth;
            pbgBmpInfo.bmiHeader.biHeight = cHeight;

            hbgBmpData = Win32.CreateDIBSection(default, ref pbgBmpInfo, Win32.EDIBColorMode.DIB_RGB_COLORS, out pbgBmpData, default, 0);

            BufMaxWidth = cWidth;
            BufMaxHeight = cHeight;
        }

        
        public bool SetFormSize(int iWidth, int iHeight)
        {
            switch (Rotate)
            {
                case 1:
                case 3:
                    int c = iHeight;
                    iHeight = iWidth;
                    iWidth = c;
                    break;

            }
            if (iWidth == BufWidth && iHeight == BufHeight && MustTrans == false) return (false);

            if (OrgWidth > iWidth || OrgHeight > iHeight || MustTrans == true)
            {
                BufWidth = iWidth;
                BufHeight = iHeight;
                MustTrans = true;
                return (TransBackBuffer());
            }
            else if(BufWidth < OrgWidth || BufHeight < OrgHeight)
            {
                BufWidth = OrgWidth;
                BufHeight = OrgHeight;
                MustTrans = true;
                return (TransBackBuffer());
            }
            else
                MustTrans = false;

            return (false);
        }
        
        /// <summary>
        /// バックバッファに画像を転送する
        /// SPI モードのときには HBmpData から
        /// NOSPI モードのときには hBitmap を取得してから転送を行う
        /// </summary>
        /// <returns></returns>
        private unsafe bool TransBackBuffer()
        {
            Win32.POINT[] points = new Win32.POINT[3];

            if ((Mode & EPluginMode.SPI) != 0 && Rotate == 0 && MustTrans == false)
            {
                SrcRWidth = BufWidth = ORotWidth = OrgWidth;
                SrcRHeight = BufHeight = ORotHeight = OrgHeight;

                hBmpData = hBitmap;
                pBmpData = org_pBmpData;
                pBmpInfo = org_pBmpInfo;
                return (true);
            }

            switch (Rotate)
            {
                case 0:
                case 2:
                default:
                    SrcRWidth = BufWidth;
                    SrcRHeight = BufHeight;
                    ORotWidth = OrgWidth;
                    ORotHeight = OrgHeight;
                    break;
                case 1:
                case 3:
                    SrcRWidth = BufHeight;
                    SrcRHeight = BufWidth;
                    ORotWidth = OrgHeight;
                    ORotHeight = OrgWidth;
                    break;
            }

            CheckBackBuffer(SrcRWidth, SrcRHeight);
            // 転送先を準備する
            switch (Rotate)
            {
                case 0:
                    points[0].x = 0; points[0].y = BufMaxHeight - BufHeight;
                    points[1].x = BufWidth; points[1].y = BufMaxHeight - BufHeight;
                    points[2].x = 0; points[2].y = BufMaxHeight;
                    break;
                case 1:
                    points[0].x = BufHeight; points[0].y = BufMaxHeight - BufWidth;
                    points[1].x = BufHeight; points[1].y = BufMaxHeight;
                    points[2].x = 0; points[2].y = 0 + BufMaxHeight - BufWidth;
                    break;
                case 2:
                    points[0].x = BufWidth - 1; points[0].y = BufMaxHeight - 1;
                    points[1].x = 0 - 1; points[1].y = BufMaxHeight - 1;
                    points[2].x = BufWidth - 1; points[2].y = BufMaxHeight - BufHeight - 1;
                    break;
                case 3:
                    points[0].x = 0; points[0].y = BufMaxHeight;
                    points[1].x = 0; points[1].y = BufMaxHeight - BufWidth;
                    points[2].x = BufHeight; points[2].y = BufMaxHeight;
                    break;
            }
            IntPtr hdc, sDC;

            // 転送元の準備
            hdc = Win32.GetWindowDC(default);
            sDC = Win32.CreateCompatibleDC(hdc);
            Win32.SelectObject(sDC, hBitmap);
            Win32.DeleteDC(hdc);

            // 転送先の準備
            hdc = Win32.GetWindowDC(default);
            IntPtr dDC = Win32.CreateCompatibleDC(hdc);
            Win32.SelectObject(dDC, hbgBmpData);
            Win32.DeleteDC(hdc);

            // 転送
            Win32.EStretchBltMode OldStretchMode = Win32.SetStretchBltMode(dDC, Win32.EStretchBltMode.STRETCH_HALFTONE);

            bool result = Win32.PlgBlt(dDC, points,
                                sDC,
                                0, 0, OrgWidth, OrgHeight,
                                default, 0, 0);

            int value;
            if (result == false)
            {
                value = Marshal.GetLastWin32Error();
                Win32.DeleteDC(sDC);
                Win32.DeleteDC(dDC);
                return (false);
            }

            Win32.SetStretchBltMode(dDC, OldStretchMode);

            Win32.DeleteDC(sDC);
            Win32.DeleteDC(dDC);

            hBmpData = hbgBmpData;
            pBmpData = pbgBmpData;
            pBmpInfo = pbgBmpInfo;

            MustTrans = false;

            return (true);
        }

        public bool AbsoluteRotate(int Value)
        {
            if (Showing == false) return (true);
            Rotate = Value;
            return (TransBackBuffer());
        }

        public bool OffsetRotate(int Value)
        {
            if (Showing == false) return (true);

            Rotate += Value;

            while (Rotate < 0) Rotate += 4;
            while (Rotate > 3) Rotate -= 4;

            return (TransBackBuffer());
        }

        public bool PauseGIFAnimate()
        {
            if (GIFAnimate == false) return (false);
            PausedTGT = Win32.timeGetTime();
            return (true);
        }
        public bool RestartGIFAnimate()
        {
            if (GIFAnimate == false) return (false);
            PreTGT += Win32.timeGetTime() - PausedTGT;
            return (true);
        }

        public unsafe bool GIFAnimateUpDateFrame(bool FrameSkip)
        {
            bool lockTaken = false;
            bool Result = true;
            try
            {
                while (true)
                {
                    Monitor.Enter(syncObject, ref lockTaken); // ロック取得

                    if (LoopCount <= LoopIndex && LoopCount > 0)
                    {
                        Result = false;
                        break;
                    }

                    FrameIndex++;
                    if (FrameIndex >= FrameCount)
                    {
                        LoopIndex++;

                        DropFrame = DropCount;
                        DropCount = 0;

                        if (LoopCount == LoopIndex)
                        {
                            Result = false;
                            break;
                        }

                        FrameIndex = 0;
                    }

                    if (GIFAnimate == false)
                    {
                        Result = false;
                        break;
                    }
                    Win32.DeleteObject(hBitmap);

                    NowTGT = Win32.timeGetTime();

                    int D = (int)(NowTGT - PreTGT) - DelayTime;
                    PreTGT = NowTGT;

                    DelayTime = Delay[FrameIndex] - D;
                    if (FrameSkip == true)
                    {
                        while (DelayTime < 0)
                        {
                            FrameIndex++;
                            DropCount++;

                            if (FrameIndex >= FrameCount)
                            {
                                DropFrame = DropCount;
                                DropCount = 0;
                                LoopIndex++;
                                if (LoopCount == LoopIndex)
                                {
                                    FrameIndex--;
                                    break;
                                }
                                FrameIndex = 0;
                            }
                            DelayTime += Delay[FrameIndex];
                        }
                    }
                    else
                    {
                        if(DelayTime < 0)DelayTime = 17;
                    }

                    ImageDNET.SelectActiveFrame(FrameDimension.Time, FrameIndex);
                    BitmapDNET = new Bitmap(ImageDNET);

                    hBitmap = BitmapDNET.GetHbitmap();

                    MustTrans = true;
                    TransBackBuffer();
                    break;
                }
            }
            finally
            {
                if (lockTaken)
                {
                    Monitor.Exit(syncObject); // ロック解放
                }
            }

            return (Result);
        }

        private int CheckOrientation()
        {
            //            if (BitmapDNET.RawFormat.Equals(ImageFormat.Jpeg))

            try
            {
                byte[] buf = ImageDNET.GetPropertyItem(0x0112).Value;
                switch (buf[0])
                {
                    default:
                        return(0);
                    case 6:
                        return (1);
                    case 3:
                        return (2);
                    case 8:
                        return (3);
                }
            }
            catch
            {
                return (0);
            }
//            return (0);
        }

        public unsafe short CheckOrientation(byte* pTemp, int Size)
        {
            short Value = CheckOrientationExif(pTemp, Size);
            short[] Table = { 0, 0, 0, 2, 0, 0, 1, 0, 3 };
            if (Value < 0 || Value > 8) return (0);
            return (Table[Value]);
        }


        private unsafe short CheckOrientationExif(byte* pTemp, int Size)
        {
            uint Int, p;
            ushort SInt;
            ushort USInt;
            short Result = 0;

            int TMode = 0;
            uint Ofs = 0;
            uint i;


            if (Size < 256) return (0);

            if (pTemp[0] != 0xff || pTemp[1] != 0xd8) return (0);
            if (pTemp[2] != 0xff) return (0);

            p = (uint)(pTemp[4] * 256 + pTemp[5] + 4);

            if (Size < p) return (0);

            if (pTemp[p] != 0xff) return (0);

            if (pTemp[2] == 0xff && pTemp[3] == 0xe1)
            {
                byte[] btemp = { 0x45, 0x78, 0x69, 0x66, 0, 0 };
                if (Acfc.MemNCmp(pTemp + 6, btemp, 6) == 0)
                    Ofs = 12;
                else
                    return (0);
            }
            else
                return (0);

            //---------------------------------------------

            for (i = Ofs; i < Size; i++)
            {
                //		SInt = *(short *)(void *)(pTemp + i + 2);
                //		if(pTemp[i] == 0x49 && pTemp[i + 1] == 0x49 && SInt == 0x2a){TMode = 1; break;}
                //		if(pTemp[i] == 0x4d && pTemp[i + 1] == 0x4d && SInt == 0x2a00){TMode = 2; break;}
                if (pTemp[i] == 0x49 && pTemp[i + 1] == 0x49) { TMode = 1; break; }
                if (pTemp[i] == 0x4d && pTemp[i + 1] == 0x4d) { TMode = 2; break; }
            }
            if (TMode == 0) goto setorientationlefttop_break;


            //---------------------------------------------

            Int = *(uint*)(void*)(pTemp + i + 4);
            Ofs = i;

            Acfc.Endian(&Int, 4, TMode);
            Ofs += Int;
            if (Size < Ofs) return (0);

            //-------------------------------------------------------------------------------------
            // Orientation を探す
            //-------------------------------------------------------------------------------------
            SInt = *(ushort*)(void*)(pTemp + Ofs);
            Ofs += 2;
            Acfc.Endian(&SInt, 2, TMode);
            i = SInt;

            while (i > 0)
            {
                if (Size < Ofs + 8) return (0);
                USInt = *(ushort*)(void*)(pTemp + Ofs);
                Acfc.Endian(&USInt, 2, TMode);

                switch (USInt)
                {
                    case 0x0112:
                        Result = *(short*)(void*)(pTemp + Ofs + 8);
                        Acfc.Endian(&Result, 2, TMode);
                        return (Result);
                    default:
                        break;
                }

                Ofs += 12;
                i--;
            }

        setorientationlefttop_break:
            return (Result);
        }

        private bool CheckGIFAnimData()
        {
            FrameCount = ImageDNET.GetFrameCount(FrameDimension.Time);
            byte[] buf;
            try
            {
                buf = ImageDNET.GetPropertyItem(0x5100).Value;
            }
            catch
            {
                return (false);
            }
            Delay = new int[FrameCount];

            for (int i = 0; i < FrameCount; i++)
            {
                int offset = i * 4;
                Delay[i] = buf[offset] | buf[offset + 1] << 8 | buf[offset + 2] << 16 | buf[offset + 3] << 24;
                Delay[i] *= 10;
                if (Delay[i] == 0) Delay[i] = 100;
            }

            buf = ImageDNET.GetPropertyItem(0x5101).Value;

            LoopCount = buf[0] | buf[1] << 8;
            LoopIndex = 0;
            FrameIndex = 0;
            DropFrame = 0;
            DropCount = 0;
            DelayTime = Delay[0];

            ImageDNET.SelectActiveFrame(FrameDimension.Time, 0);
            PreTGT = NowTGT = Win32.timeGetTime();
            return (true);
        }

        public unsafe bool SetShowSubIndex(ref ImageInfo SrcImageInfo, int SubIndex, CallbackDelegate ProgressCallback)
        {
            byte[] bFileName = Acfc.ANSIAddNullStr(Encoding.GetEncoding("Shift_JIS").GetBytes(ImageInfo.FileName));
            int DataLen = Marshal.SizeOf(typeof(byte)) * Acfc.StrLen(bFileName) + 1;
            IntPtr SFileName = Marshal.AllocCoTaskMem(DataLen);
            Marshal.Copy(bFileName, 0, SFileName, DataLen);

            if (nowArchivedFileInfo[SubIndex].FileSize > 0)
            {
                if (ArchiveSpi.GetFile(SFileName, (int)nowArchivedFileInfo[SubIndex].Position, out HFileImage, 0x0100,
                                                                    default, 0
                                                                    ) != 0) return (false);


                pFileImage = Win32.LocalLock(HFileImage);

                ArchivedFileInfo tempAFI = nowArchivedFileInfo[SubIndex];

                string tPath = Encoding.GetEncoding("SJIS").GetString(tempAFI.Path, Acfc.StrLen(tempAFI.Path));
                string tFileName = Encoding.GetEncoding("SJIS").GetString(tempAFI.FileName, Acfc.StrLen(tempAFI.FileName));

                string AFileName = tPath + tFileName;
                SrcImageInfo.FileName = AFileName;
                SrcImageInfo.FileSize = tempAFI.FileSize;
                SrcImageInfo.Timestamp = DateTimeOffset.FromUnixTimeSeconds(tempAFI.Timestamp).ToLocalTime(); 

                if (CheckFileType(ref SrcImageInfo, pFileImage, nowArchivedFileInfo[SubIndex].FileSize))
                {
                    if (LoadFromFileInMemory(ref SrcImageInfo, pFileImage, nowArchivedFileInfo[SubIndex].FileSize, ProgressCallback) == true)
                    {
                        Clear(EPluginMode.ACVINNER);
                        Mode |= EPluginMode.ACVINNER;
                        org_HFileImage = HFileImage;
                        org_pFileImage = pFileImage;
                        return (true);
                    }
                }
                Win32.LocalUnlock(HFileImage);
                Win32.LocalFree(HFileImage);
                Marshal.FreeHGlobal(SFileName);
            }

            return (false);
        }


        public unsafe bool GetArchiveFileLists(ref List<ImageInfo> SubFileLists) // ファイルリストを取得
        {
            int i;
            SubFileLists.Clear();
            for (i = 0; i < nowArchivedFileInfo.Count; i++)
            {
                ArchivedFileInfo tempAFI = nowArchivedFileInfo[i];

                string tPath = Encoding.GetEncoding("SJIS").GetString(tempAFI.Path, Acfc.StrLen(tempAFI.Path));
                string tFileName = Encoding.GetEncoding("SJIS").GetString(tempAFI.FileName, Acfc.StrLen(tempAFI.FileName));

                ImageInfo NewII = new ImageInfo();

                NewII.Timestamp = Acfc.FromUnixTime(nowArchivedFileInfo[i].Timestamp);
                NewII.FileSize = nowArchivedFileInfo[i].FileSize;
                NewII.Rotate = -1;
                NewII.FileName = tPath + tFileName; // Encoding.Unicode.GetString(cTemp);
                SubFileLists.Add(NewII);
            }
            return (true);
        }

        public unsafe bool SetSubImageFile(ref ImageInfo Src)
        {
            int i;

            for (i = 0; i < ImageNum; i++)
            {
                ArchivedFileInfo tempAFI = nowArchivedFileInfo[i];

                string tPath = Encoding.GetEncoding("SJIS").GetString(tempAFI.Path, Acfc.StrLen(tempAFI.Path));
                string tFileName = Encoding.GetEncoding("SJIS").GetString(tempAFI.FileName, Acfc.StrLen(tempAFI.FileName));

                if (Src.FileName == tPath + tFileName) break;
            }
            if (i == ImageNum) return (false);
            bool Result = SetShowSubIndex(ref Src, i, ProgressCallback);
            SubImageInfo = Src;
            return (Result);
        }

        public bool SetSubImageFile(List<ImageInfo> FileLists, ref int i, int Ofs)
        {
            while (FileLists.Count > 0)
            {
                ImageInfo tempII = FileLists[i];
                if (SetSubImageFile(ref tempII) == true)
                {
                    FileLists[i] = tempII;
                    return (true);
                }

                FileLists.RemoveAt(i);

                if (Ofs < 0) i += Ofs;
                if (i < 0) break;
                if (i >= FileLists.Count) break;
            }
            return (false);
        }

        // 画像データをクリアする
        public void Clear(EPluginMode DelMode)
        {
            switch ((Mode & DelMode & EPluginMode.PICTURE))
            {
                case EPluginMode.DNET:
                    if (hBitmap != default)
                    {
                        Win32.DeleteObject(hBitmap);
                        hBitmap = default;
                    }

                    if (BitmapDNET != default)
                    {
                        BitmapDNET.Dispose();
                        BitmapDNET = default;
                    }

                    if(ImageDNET != default)
                    {
                        ImageDNET.Dispose();
                        ImageDNET = default;
                    }

                    if(memoryStream != default)
                    {
                        memoryStream.Dispose();
                        memoryStream = default;
                    }
                    break;

                case EPluginMode.SPI:
                    if (hBitmap != default)
                    {
                        Win32.DeleteObject(hBitmap);
                        hBitmap = default;
                    }

                    if (org_HBmpData != default)
                    {
                        Win32.GlobalUnlock(org_HBmpData);
                        Win32.GlobalFree(org_HBmpData);
                        org_HBmpData = default;
                        org_pBmpData = default;
                    }

                    if (org_HBmpInfo != default)
                    {
                        Win32.GlobalUnlock(org_HBmpInfo);
                        Win32.GlobalFree(org_HBmpInfo);
                        org_HBmpInfo = default;
                        org_pBmpInfo = default;
                    }
                    break;
            }

            if((DelMode & EPluginMode.PICTURE) != 0)
            {
                OrgWidth = 0;
                OrgHeight = 0;
            }

            if ((Mode & DelMode & EPluginMode.ARCHIVE) != 0)
            {
                if (oldArchivedFileInfo != default)
                {
                    oldArchivedFileInfo.Clear();
                    oldArchivedFileInfo = default;
                }
                ImageInfo = new ImageInfo();
                SubImageInfo = new ImageInfo();
            }

            if ((Mode & DelMode & EPluginMode.ACVINNER) != 0)
            {
                if (org_HFileImage != default)
                {
                    Win32.LocalUnlock(org_HFileImage);
                    Win32.LocalFree(org_pFileImage);
                    org_HFileImage = default;
                    org_pFileImage = default;
                }
                SubImageInfo = new ImageInfo();
            }

            if ((DelMode & EPluginMode.CLEARFILE) != 0)
            {
                ImageInfo = new ImageInfo();
                SubImageInfo = new ImageInfo();
            }

            Mode &= (EPluginMode.ALL ^ DelMode);
        }

        private void DeleteBackBuffer()
        {
            if (hbgBmpData != default)
            {
                Win32.DeleteObject(hbgBmpData);
                hbgBmpData = default;
                BufMaxWidth = 0;
                BufMaxHeight = 0;
            }
        }

        public void OpenSpiSettingDialog(IntPtr hwnd)
        {
            if (NowSpi.ConfigurationDlg != default)
                NowSpi.ConfigurationDlg(hwnd, 1);
            else
                return;
        }


        private Bitmap DuplicateImage(int newWidth, int newHeight)
        {
            if ((Mode & EPluginMode.ALL) == 0) return (null);

            Bitmap bmp = new Bitmap(newWidth, newHeight);
            Graphics g = Graphics.FromImage(bmp);


            // 転送先の準備
            IntPtr hdc = g.GetHdc();

            IntPtr sDC = Win32.CreateCompatibleDC(hdc);
            IntPtr hTargetBmp = bmp.GetHbitmap();
            Win32.SelectObject(sDC, hBitmap);


            Win32.EStretchBltMode OldStretchMode = Win32.SetStretchBltMode(hdc, Win32.EStretchBltMode.STRETCH_HALFTONE);
            bool res = Win32.StretchBlt(hdc,
                                0, 0, newWidth, newHeight,
                                sDC,
                                0, 0, OrgWidth, OrgHeight,
                                Win32.ETernaryRasterOperations.SRCCOPY);

            Win32.SetStretchBltMode(hdc, OldStretchMode);

            Win32.DeleteDC(sDC);
            Win32.DeleteObject(hTargetBmp);

            g.ReleaseHdc();

            return (bmp);
        }

        /// <summary>
        /// 画像をクリップボードにコピー
        /// </summary>
        /// <returns></returns>
        public bool CopyImageToClipboard()
        {
            Bitmap bmp = DuplicateImage(OrgWidth, OrgHeight);
            Clipboard.SetImage(bmp);

            bmp.Dispose();

            return (true);
        }

        public bool SaveJpeg(string svFileName, int svWidth, int svHeight, int svCompLevel)
        {
            Bitmap bmp = DuplicateImage(svWidth, svHeight);

            ImageCodecInfo jpegEncoder = default;
            foreach (ImageCodecInfo ici in ImageCodecInfo.GetImageEncoders())
            {
                if (ici.FormatID == ImageFormat.Jpeg.Guid)
                {
                    jpegEncoder = ici;
                    break;
                }
            }
            
            if (jpegEncoder != default)
            {

                EncoderParameter encParam = new EncoderParameter(System.Drawing.Imaging.Encoder.Quality, svCompLevel);
                EncoderParameters encParams = new EncoderParameters(1);
                encParams.Param[0] = encParam;

                bmp.Save(svFileName, jpegEncoder, encParams);
            }

            bmp.Dispose();

            return (true);
        }

        public bool SavePNG(string svFileName, int svWidth, int svHeight)
        {
            Bitmap bmp = DuplicateImage(SrcRWidth, SrcRHeight);

            bmp.Save(svFileName, ImageFormat.Png);

            bmp.Dispose();

            return (true);
        }

    }


}

