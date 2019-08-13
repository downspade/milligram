
	spi_wrapper rev0.06 ソース+サンプルコードセットディレクトリ構成

					Author: kana(inokuchi@mvg.biglobe.ne.jp)

bin/			サンプルプログラムをコンパイルしたものが入っています
doc/			ドキュメントとたわ言が書かれたファイルが入ってます
   spi_wrapper.txt	spi_wrapper、サンプルプログラム、配布条件について書か
			れています
   tips.txt		Plug-inを使う時に出くわす問題に関する注意点を少し書い
			てみました。
   dialy.txt		たわ言です。
spiif_rev6/		Visual C++6.0でのソースコード一式です。
 |			$(root)がwrapperのソース、各ディレクトリはそれぞれサ
 |			ンプルプログラムです。
 +spiany/		特定のPlug-inに対して特定の入力ファイルを指定した場合
 |			の挙動を調査するサンプル。
 +cnvbmp/		Susieのsendtoなどに入れて指定のファイルをBitmapに変換
 |			するサンプル。
 +tobitmap/		SusieやエクスプローラからD&DしたファイルをBitmapに変
			換するサンプル。
