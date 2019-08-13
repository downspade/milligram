// メッセージボックスが表示されない問題

namespace milligram
{
    public partial class MainForm : Form
    {
        bool GetTransRect(Rectangle wRect, ref Rectangle Src, ref Rectangle Dest)
        {
            Rectangle clip, temp;
            double dl, dt, dr, db;
            double sl, st, sr, sb;
            double rt;

            Src = new Rectangle(0, 0, Susie.SrcRWidth, Susie.SrcRHeight);
            Dest = wRect;
            rt = (float)Dest.Width / Src.Width;

            if (FullScreen == true)
            {
                clip = ClientRectangle;
                clip.X = Left;
                clip.Y = Top;
            }
            else
            {
                clip = Desktop;
            }

            //上下左右何ピクセルカットされたかを得る
            temp = Dest;
            temp.Intersect(clip);

            dl = temp.Left - Dest.Left;
            dr = Dest.Right - temp.Right;
            dt = temp.Top - Dest.Top;
            db = Dest.Bottom - temp.Bottom;

            // ソースから何ピクセルカットするかを得る
            sl = Math.Round(dl / rt);
            sr = Math.Round(dr / rt);
            st = Math.Round(dt / rt);
            sb = Math.Round(db / rt);

            // 上下左右に何ピクセル戻すかを計算する
            dl = Math.Round(sl * rt);
            dr = Math.Round(sr * rt);
            dt = Math.Round(st * rt);
            db = Math.Round(sb * rt);

            // それぞれソースに反映
            Src.X += (int)sl;
            Src.Width -= (int)sl;
            Src.Y += (int)st;
            Src.Height -= (int)st;
            Src.Width -= (int)sr;
            Src.Height -= (int)sb;

            Dest.X += (int)dl;
            Dest.Width -= (int)dl;
            Dest.Y += (int)dt;
            Dest.Height -= (int)dt;
            Dest.Width -= (int)dr;
            Dest.Height -= (int)db;

            return (true);
        }



        private void MainForm_Paint(object sender, PaintEventArgs e)
        {
            if (EnableDraw > 0 || ShowingList == true) return;
            BeginUpdate();

            if (Susie.Mode != EPluginMode.NONE) // 画像が描画されている場合
            {
                IntPtr hDC = e.Graphics.GetHdc();
                // 画像表示
                Win32.EStretchBltMode OldStretchMode = Win32.SetStretchBltMode(hDC, Win32.EStretchBltMode.STRETCH_HALFTONE);

                Rectangle Src = new Rectangle();
                Rectangle Dest = new Rectangle();
                Rectangle wRect = new Rectangle(WLeft, WTop, WWidth, WHeight);

                Point offset = new Point(0, 0);

                if (FullScreen == false)
                {
                    offset = new Point(-RLeft, -RTop);

                    if (WLeft >= FrameWidth)
                        offset.X += FrameWidth;
                    else if (WLeft > 0)
                        offset.X += WLeft;

                    if (WTop >= FrameWidth)
                        offset.Y += FrameWidth;
                    else if (WTop > 0)
                        offset.Y += WTop;

                }
                else
                {
                    offset = new Point(-MLeft + FrameWidth, -MTop + FrameWidth);
                }


                GetTransRect(wRect, ref Src, ref Dest);
                Dest.Offset(offset);

                Win32.StretchDIBits(hDC,
                                    Dest.Left, Dest.Top, Dest.Width, Dest.Height,
                                    Src.Left, Susie.SrcRHeight - (Src.Top + Src.Height), Src.Width, Src.Height,
                                    Susie.pBmpData, ref Susie.pBmpInfo,
                                    Win32.EDIBColorMode.DIB_RGB_COLORS, Win32.ETernaryRasterOperations.SRCCOPY);

                Graphics g = this.CreateGraphics();
                if (FrameWidth > 0)
                {
                    SolidBrush brush = new SolidBrush(FrameColor);

                    Rectangle[] fRect = new Rectangle[4];
                    fRect[0] = new Rectangle(WLeft - FrameWidth, WTop - FrameWidth, WWidth + FrameWidth * 2, FrameWidth);
                    fRect[1] = new Rectangle(WLeft - FrameWidth, WTop, FrameWidth, WHeight);
                    fRect[2] = new Rectangle(WLeft + WWidth, WTop, FrameWidth, WHeight);
                    fRect[3] = new Rectangle(WLeft - FrameWidth, WTop + WHeight, WWidth + FrameWidth * 2, FrameWidth);

                    foreach (var a in fRect)
                    {
                        a.Offset(offset);
                        g.FillRectangle(brush, a);
                    }
                    brush.Dispose();
                }

                if (FullScreen == true)
                {
                    SolidBrush brush = new SolidBrush(FullFillColor);

                    Dest.X -= FrameWidth;
                    Dest.Y -= FrameWidth;
                    Dest.Width += FrameWidth * 2;
                    Dest.Height += FrameWidth * 2;

                    Rectangle[] fRect = new Rectangle[4];

                    fRect[0] = new Rectangle(0, 0, Width, Dest.Top);
                    fRect[1] = new Rectangle(0, Dest.Bottom, Width, Bottom - Dest.Bottom);
                    fRect[2] = new Rectangle(0, Dest.Top, Dest.Left, Dest.Height);
                    fRect[3] = new Rectangle(Dest.Right, Dest.Top, Right, Dest.Height);

                    foreach (var a in fRect)
                    {
                        if (a.IntersectsWith(ClientRectangle))
                        {
                            a.Intersect(ClientRectangle);
                            g.FillRectangle(brush, a);
                        }
                    }
                    brush.Dispose();
                }
                e.Graphics.ReleaseHdc(hDC);
                Win32.SetStretchBltMode(hDC, OldStretchMode);


                if (GIFRefresh == true)
                {
                    GIFRefresh = false;
                    if (Susie.GIFAnimateUpDateFrame(EnableDropFrame) == false) // 次のフレームの準備をする
                    {
                        EndGIFAnimeThread();
                    }

                }
            }
            else  // 画像が描画されていない場合
            {
                // 画像表示
                Rectangle Dest = new Rectangle(RLeft, RTop, RWidth, RHeight);
                Rectangle wRect = new Rectangle(WLeft, WTop, WWidth, WHeight);

                Point offset = new Point(0, 0);

                if (FullScreen == false)
                {
                    offset = new Point(-RLeft, -RTop);

                    if (WLeft >= FrameWidth)
                        offset.X += FrameWidth;
                    else if (WLeft > 0)
                        offset.X += WLeft;

                    if (WTop >= FrameWidth)
                        offset.Y += FrameWidth;
                    else if (WTop > 0)
                        offset.Y += WTop;

                }
                else
                {
                    offset = new Point(-MLeft + FrameWidth, -MTop + FrameWidth);
                }

                Dest.Offset(offset);

                Graphics g = this.CreateGraphics();

                SolidBrush brush = new SolidBrush(FullFillColor);

                if (FullScreen == true)
                {
                    Pen pen = new Pen(DrawColor);
                    g.DrawRectangle(pen, Dest.Left, Dest.Top, Dest.Width - 1, Dest.Height - 1);
                    g.FillRectangle(brush, Dest.Left + 1, Dest.Top + 1, Dest.Width - 2, Dest.Height - 2);
                    pen.Dispose();
                }
                else
                    g.FillRectangle(brush, Dest.Left, Dest.Top, Dest.Width, Dest.Height);

                if (FrameWidth > 0)
                {
                    brush.Color = FrameColor;

                    Rectangle[] fRect = new Rectangle[4];
                    fRect[0] = new Rectangle(WLeft - FrameWidth, WTop - FrameWidth, WWidth + FrameWidth * 2, FrameWidth);
                    fRect[1] = new Rectangle(WLeft - FrameWidth, WTop, FrameWidth, WHeight);
                    fRect[2] = new Rectangle(WLeft + WWidth, WTop, FrameWidth, WHeight);
                    fRect[3] = new Rectangle(WLeft - FrameWidth, WTop + WHeight, WWidth + FrameWidth * 2, FrameWidth);

                    foreach (var a in fRect)
                    {
                        a.Offset(offset);
                        g.FillRectangle(brush, a);
                    }
                }

                if (FullScreen == true)
                {
                    brush.Color = FullFillColor;

                    Dest.X -= FrameWidth;
                    Dest.Y -= FrameWidth;
                    Dest.Width += FrameWidth * 2;
                    Dest.Height += FrameWidth * 2;

                    Rectangle[] fRect = new Rectangle[4];

                    fRect[0] = new Rectangle(0, 0, Width, Dest.Top);
                    fRect[1] = new Rectangle(0, Dest.Bottom, Width, Bottom - Dest.Bottom);
                    fRect[2] = new Rectangle(0, Dest.Top, Dest.Left, Dest.Height);
                    fRect[3] = new Rectangle(Dest.Right, Dest.Top, Right, Dest.Height);

                    foreach (var a in fRect)
                    {
                        if (a.IntersectsWith(ClientRectangle))
                        {
                            a.Intersect(ClientRectangle);
                            g.FillRectangle(brush, a);
                        }
                    }
                }
                brush.Dispose();
            }

            // 読み込みラインの表示
            //if (LoadState == 1)
            //{
            //    Graphics g = this.CreateGraphics();
            //    SolidBrush brush = new SolidBrush(FullFillColor);

            //    brush.Color = DrawColor;
            //    int barWidth = (int)(ProgressRatio * WWidth);
            //    Point offset = new Point(0, 0);
            //    if (FullScreen == false)
            //    {
            //        if (WLeft >= FrameWidth)
            //            offset.X += FrameWidth;
            //        else
            //            offset.X += WLeft;

            //        if (WTop >= FrameWidth)
            //            offset.Y += FrameWidth;
            //        else
            //            offset.Y += WTop;


            //        g.FillRectangle(brush, offset.X, WHeight - 2 + offset.Y, barWidth, 2);
            //    }
            //    else
            //    {
            //        g.FillRectangle(brush, WLeft - MLeft, WTop + WHeight - 2 - MLeft, barWidth, 2);
            //    }

            //    brush.Dispose();
            //    g.Dispose();
            //}

            if (SSIcon >= 0) DrawSSIcon();
            EndUpdate();
        }


        // ウィンドウ関係



        // ウィンドウ上の相対位置を画像上の相対位置に計算し直す
        // 回転や拡大縮小は考慮しない
        Point GetLocalPoint(Point Src)
        {
            Point Dest = new Point();
            if (FullScreen == false)
            {
                Dest.X = Src.X + FrameWidth + WLeft - RLeft;
                Dest.Y = Src.Y + FrameWidth + WTop - RTop;
            }
            else
            {
                Dest.X = Src.X + WLeft;
                Dest.Y = Src.Y + WTop;
            }
            return (Dest);
        }

        /// ウィンドウ上の相対位置を画像上の相対位置に計算し直す 
        /// 回転や拡大縮小は考慮しない
        void GetLocalPoint(int &X, int &Y)
        {
            if (FullScreen == false)
            {
                X = X + FrameWidth + WLeft - RLeft;
                Y = Y + FrameWidth + WTop - RTop;
            }
            else
            {
                X = X + WLeft;
                Y = Y + WTop;
            }
        }

        //----------------------------------------------------------------------------------------------------------
        //
        //
        //
        //----------------------------------------------------------------------------------------------------------


        private void MainForm_KeyDown(object sender, KeyEventArgs e)
        {
            // 18 37 戻る
            // 18 39 進む
            if (progressForm.Visible) return;

            if (ShowingList == true)
            {
                DisplayBox_KeyDown(sender, e);
                return;
            }

            if (Locked) return;

            int SubKey = 0;

            if (((Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_LSHIFT) & 0x8000) | (Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_RSHIFT) & 0x8000)) != 0) SubKey |= 1;
            if (((Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_LCONTROL) & 0x8000) | (Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_RCONTROL) & 0x8000)) != 0) SubKey |= 2;
            if (((Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_LMENU) & 0x8000) | (Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_RMENU) & 0x8000)) != 0) SubKey |= 4;

            switch (e.KeyCode)
            {
                case Keys.Space:
                    if (SubKey == 0)
                        ShowOffsetImage(1);
                    else if (SubKey == 1)
                        ShowOffsetImage(-1);
                    else if (SubKey == 2)
                        MnToggleVisible_Click(null, null);
                    break;

                case Keys.Down:
                    ShowOffsetImage(1);
                    break;
                case Keys.Up:
                    ShowOffsetImage(-1);
                    break;
                case Keys.Next:
                    ShowOffsetImage(10);
                    break;
                case Keys.Prior:
                    ShowOffsetImage(-10);
                    break;
                case Keys.Home:
                    ShowAbsoluteImage(0, 1);
                    break;
                case Keys.End:
                    ShowAbsoluteImage(DisplayLists.Count - 1, -1);
                    break;

                case Keys.Return:
                    ToggleShowList(EShowMode.LIST);
                    break;

                case Keys.Delete:
                    if (SubKey == 0)
                        DeleteFileLists(0);
                    else if (SubKey == 2)
                        DeleteFileLists(1);
                    else if (SubKey == 3)
                        DeleteFileLists(2);
                    break;

                case Keys.F5:
                    ShowAbsoluteImage(ShowIndex, 0);
                    break;

                case Keys.Oemplus:
                case Keys.Add:
                    ZoomImage(Percentage * 1.5);
                    break;
                case Keys.OemMinus:
                case Keys.Subtract:
                    ZoomImage(Percentage / 1.5);
                    break;
                case Keys.Multiply:
                case Keys.Oem5:
                    MnInScreen_Click(null, null);
                    break;

                case Keys.O:
                    MnOpenFile_Click(null, null);
                    break;

                case Keys.N:
                    if (SubKey == 2)
                        MnFileRename_Click(null, null);
                    break;

                case Keys.Oem2:
                    MnShowPbyP_Click(null, null);
                    break;
                case Keys.Divide:
                    MnShowPbyP_Click(null, null);
                    break;
                case Keys.A:
                    MnShowPbyP_Click(null, null);
                    break;

                case Keys.S:
                    if (SubKey == 0)
                        MnInScreen_Click(null, null);
                    else if (SubKey == 2)
                        MnSave_Click(null, null);
                    else if (SubKey == 1)
                        MnJpegSave_Click(null, null);
                    else if (SubKey == 3)
                        MnJpegSaveShowingSize_Click(null, null);
                    break;

                case Keys.C:
                    if (SubKey == 0)
                        MnCenter_Click(null, null);
                    else if (SubKey == 1)
                        MnCopyImage_Click(null, null);
                    else if (SubKey == 2)
                        MnFileCopy_Click(null, null);
                    else if (SubKey == 3)
                        MnCopyFilePath_Click(null, null);
                    break;

                case Keys.T:
                    if (SubKey == 0)
                        MnAlwaysTop_Click(null, null);
                    else if (SubKey == 2)
                        SortFileLists(ESortType_NAME);
                    break;

                case Keys.B:
                    MnFixRatio_Click(null, null);
                    break;

                case Keys.W:
                    MnFitToScreen_Click(null, null);
                    break;
                case Keys.H:
                    MnUseWholeScreen_Click(null, null);
                    break;
                case Keys.F:
                    MnFullScreen_Click(null, null);
                    break;

                case Keys.D:
                    MnSlideShow_Click(null, null);
                    break;
                case Keys.R:
                    if (SubKey == 0)
                        ShowAbsoluteImage(ShowIndex, 0);
                    else if (SubKey == 2)
                        SortFileLists(ESortType_RANDOM);
                    break;

                case Keys.X:
                    if (SubKey == 2)
                        MnFileCut_Click(null, null);
                    break;

                case Keys.Back:
                    MnCloseArchive_Click(null, null);
                    break;

                case Keys.V:
                    if (SubKey == 2)
                        MnFilePaste_Click(null, null);
                    break;

                case Keys.I:
                    MnShowInformation_Click(null, null);
                    break;

                case Keys.P:
                    if (SubKey == 2)
                        MnOpenExistsFolder_Click(null, null);
                    else
                        MnOpenFolder_Click(null, null);
                    break;

                case Keys.G:
                    if (SubKey == 2)
                        SortFileLists(ESortType_TIMESTAMP);
                    else
                        MnFixDiagonalLength_Click(null, null);
                    break;

                case Keys.E:
                    if (SubKey == 2)
                        SortFileLists(ESortType_EXT);
                    break;

                case Keys.Y:
                    if (SubKey == 2)
                        SortFileLists(ESortType_FILESIZE);
                    break;

                case Keys.U:
                    if (SubKey == 2)
                        SortFileLists(ESortType_REVERSE);
                    break;

                case Keys.M:
                    MnMoveFile_Click(null, null);
                    break;

                case Keys.Oemcomma:
                    OffsetRotate(-1);
                    break;

                case Keys.OemPeriod:
                    OffsetRotate(1);
                    break;

                case Keys.Escape:
                    Close();
                    break;
            }

        }

 
        private void DisplayBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (ShowingList == false)
            {
                MainForm_KeyDown(sender, e);
                return;
            }

            if (Locked) return;

            int SubKey = 0;

            if (((Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_LSHIFT) & 0x8000) | (Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_RSHIFT) & 0x8000)) != 0)
                SubKey |= 1;

            if (((Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_LCONTROL) & 0x8000) | (Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_RCONTROL) & 0x8000)) != 0)
                SubKey |= 2;

            if (((Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_LMENU) & 0x8000) | (Win32.GetAsyncKeyState(Win32.EVirtualKey.VK_RMENU) & 0x8000)) != 0)
                SubKey |= 4;

            switch (e.KeyCode)
            {
                case Keys.Space:
                    if (SubKey == 2)
                        MnToggleShow_Click(null, null);
                    break;

                case Keys.Return:
                    ToggleShowList(EShowMode.PICTURE);
                    break;

                case Keys.Delete:
                    if (SubKey == 0)
                        DeleteFileLists(0);
                    else if (SubKey == 2)
                        DeleteFileLists(1);
                    else if (SubKey == 3)
                        DeleteFileLists(2);
                    break;

                case Keys.Down:
                    if (SubKey == 4)
                        MoveSelectedList(1);
                    break;

                case Keys.Up:
                    if (SubKey == 4)
                        MoveSelectedList(-1);
                    break;

                case Keys.O:
                    MnOpenFile_Click(null, null);
                    break;

                case Keys.N:
                    if (SubKey == 2)
                        MnFileRename_Click(null, null);
                    break;

                case Keys.A:
                    if (SubKey == 2)
                    {
                        for (int i = 0; i < DisplayBox.Items.Count; i++)
                            DisplayBox.SetSelected(i, true);
                    }
                    break;

                case Keys.S:
                    if (SubKey == 0)
                        MnInScreen_Click(null, null);
                    else if (SubKey == 2)
                        MnSave_Click(null, null);
                    break;

                case Keys.C:
                    if (SubKey == 0)
                        MnCenter_Click(null, null);
                    else if (SubKey == 2)
                        MnFileCopy_Click(null, null);
                    else if (SubKey == 3)
                        MnCopyFilePath_Click(null, null);
                    break;

                case Keys.T:
                    if (SubKey == 0)
                        MnAlwaysTop_Click(null, null);
                    else if (SubKey == 2)
                        SortFileLists(ESortType_NAME);
                    break;

                case Keys.F:
                    MnFullScreen_Click(null, null);
                    break;

                case Keys.X:
                    if (SubKey == 2)
                        MnFileCut_Click(null, null);
                    break;

                case Keys.Back:
                    MnCloseArchive_Click(null, null);
                    break;

                case Keys.V:
                    if (SubKey == 2)
                        MnFilePaste_Click(null, null);
                    break;

                case Keys.I:
                    if (SubKey == 2)
                    {
                        for (int i = 0; i < DisplayBox.Items.Count; i++)
                            DisplayBox.SetSelected(i, !DisplayBox.GetSelected(i));
                    }
                    break;

                case Keys.D:
                    if (SubKey == 2)
                    {
                        for (int i = 0; i < DisplayBox.Items.Count; i++)
                            DisplayBox.SetSelected(i, false);
                    }
                    break;

                case Keys.P:
                    if (SubKey == 2)
                        MnOpenExistsFolder_Click(null, null);
                    else
                        MnOpenFolder_Click(null, null);
                    break;

                case Keys.G:
                    if (SubKey == 2)
                        SortFileLists(ESortType_TIMESTAMP);
                    break;

                case Keys.E:
                    if (SubKey == 2)
                        SortFileLists(ESortType_EXT);
                    break;

                case Keys.Y:
                    if (SubKey == 2)
                        SortFileLists(ESortType_FILESIZE);
                    break;

                case Keys.U:
                    if (SubKey == 2)
                        SortFileLists(ESortType_REVERSE);
                    break;

                case Keys.R:
                    if (SubKey == 2)
                        SortFileLists(ESortType_RANDOM);
                    break;

                case Keys.Escape:
                    Close();
                    break;
            }
        }

 

        //-----------------------------------------------------------------------------------------------------
        //System.Media.SystemSounds.Asterisk.Play();
        //MessageBox.Show(Properties.Resources.m1001, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
        //Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData) + Path.DirectorySeparatorChar;

    }
}

