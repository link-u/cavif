# コマンドラインオプションの説明

選択肢から選ぶものは、断りがない限り一番最初がデフォルト。

## 入出力

### 入力

 - `-i input.png`（必須）
   - 対応フォーマット
     - 1/2/4/8bit Gray
     - 8/16bit RGB
     - alpha channel対応（ただし現状無視される）

### 出力

 - `-o output.avif`（必須）

### エンコード結果の表示

 - `--show-result`
   - 初期値：表示しない

デコードした結果を表示する。ただし、現状デコーダ側へ渡される設定（OBUシーケンスヘッダ）の内容しか表示できない。

例：

```
% cavif -i hato.png -o hato.avif --show-result
[2020/01/29 02:33:26 INFO ] cavif
[2020/01/29 02:33:26 INFO ] libaom ver: 1.0.0-errata1-avif
[2020/01/29 02:33:27 INFO ] Encoding: hato.png -> hato.avif
[2020/01/29 02:35:15 INFO ] Encoding done: hato.png -> hato.avif
[2020/01/29 02:35:15 INFO ] <Encoding Result>
[2020/01/29 02:35:15 INFO ]  - OBU Sequence Header:
[2020/01/29 02:35:15 INFO ]    - AV1 Profile: 0
[2020/01/29 02:35:15 INFO ]    - Still picture: Yes
[2020/01/29 02:35:15 INFO ]    - Reduced still picture header: Yes
[2020/01/29 02:35:15 INFO ]    - Sequence Level Index at OperatingPoint[0]: 12
[2020/01/29 02:35:15 INFO ]    - Max frame width: 3082
[2020/01/29 02:35:15 INFO ]    - Max frame height: 2048
[2020/01/29 02:35:15 INFO ]    - Use 128x128 superblock: Yes
[2020/01/29 02:35:15 INFO ]    - FilterIntra enabled: Yes
[2020/01/29 02:35:15 INFO ]    - IntraEdgeFilter enabled: Yes
[2020/01/29 02:35:15 INFO ]    - Superres enabled: No
[2020/01/29 02:35:15 INFO ]    - CDEF enabled: No
[2020/01/29 02:35:15 INFO ]    - Loop Restoration enabled: No
[2020/01/29 02:35:15 INFO ]    - Film Grain Params Present: No
[2020/01/29 02:35:15 INFO ]    - Color Info:
[2020/01/29 02:35:15 INFO ]      - High bit-depth: No
[2020/01/29 02:35:15 INFO ]      - Twelve bit: No
[2020/01/29 02:35:15 INFO ]      - Monochrome: No
[2020/01/29 02:35:15 INFO ]      - Color primaries: <Unknownn>
[2020/01/29 02:35:15 INFO ]      - Transfer characteristics: <Unknownn>
[2020/01/29 02:35:15 INFO ]      - Matrix coefficients: <Unknownn>
[2020/01/29 02:35:15 INFO ]      - Color range: Limited
[2020/01/29 02:35:15 INFO ]      - Sub sampling X: 1
[2020/01/29 02:35:15 INFO ]      - Sub sampling Y: 1
[2020/01/29 02:35:15 INFO ]      - Chroma sample position: 0
[2020/01/29 02:35:15 INFO ]      - Separate UV Delta Q: No
```

## メタデータ

### 切り抜き・回転・反転

適用される順番は「切り抜き→回転→反転」

#### 切り抜き

 - `--crop-size widthN/widthD,heightN/heightD`
   - 初期値：なし。切り抜かない
 - `--crop-offset horizOffN/horizOffD,vertOffN/vertOffD`
   - 初期値：中心から切り抜く

表示時に切り抜くサイズとオフセットを分数（`N/D`）で指定する。`N/1`の時（整数）の時だけ、`N`と省略可能。

デフォルトでは指定されたサイズで中心から切り抜き、offsetが指定されたら、その分移動したところから切り抜く。

例：

 - `--crop-size 1000/3,1000/7`（333.3x142.9で中心から切り抜く）
 - `--crop-size 300,320`（300x320で中心から切り抜く）

#### 回転

 - `--rotation [0, 90, 180, 270]`
   - 初期値：回転しない

表示時に回転する。反時計回り。

#### 反転

 - `--mirror [vertical, horizontal]`
   - 初期値：反転しない

表示時に反転する。

## AV1 シーケンシャルヘッダ

### 静止画用の削減されたヘッダを出力せず、動画用のフルのヘッダを出力する

 - `--full-still-picture-header`
   - 指定しないとき：  
   `still_pisture=1`かつ`reduced_still_picture_header=1`の静止画専用ヘッダを出力する

このオプションを指定すると、動画用のヘッダを出力する。静止画専用のヘッダにくらべて、3バイトぐらい長くなる。

## プロファイル

### AV1 シーケンス・プロファイル

 - `--profile (0=base, 1=high, 2=professional)`
   - デフォルト：`0(base profile)`

AV1のプロファイルを指定する。[プロファイルごとに使えるピクセルフォーマットとbit depthが異なる](https://aomediacodec.github.io/av1-spec/#sequence-header-obu-semantics)。

<table class="table table-sm table-bordered">
  <thead>
    <tr>
      <th>seq_profile</th>
      <th>Bit depth</th>
      <th>Monochrome support</th>
      <th>Chroma subsampling</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>0</td>
      <td>8 or 10</td>
      <td>Yes</td>
      <td>YUV 4:2:0</td>
    </tr>
    <tr>
      <td>1</td>
      <td>8 or 10</td>
      <td>No</td>
      <td>YUV 4:4:4</td>
    </tr>
    <tr>
      <td>2</td>
      <td>8 or 10</td>
      <td>Yes</td>
      <td>YUV 4:2:2</td>
    </tr>
    <tr>
      <td>2</td>
      <td>12</td>
      <td>Yes</td>
      <td>YUV 4:2:0, YUV 4:2:2, YUV 4:4:4</td>
    </tr>
  </tbody>
</table>

### ピクセルフォーマット

 - `--pix-fmt [yuv420, yuv422, yuv444]`
   - 初期値：`yuv420`

出力される画像のピクセルフォーマット。

使えるビット深度とピクセルフォーマットとプロファイルの組み合わせに制限があるので注意。カラー画像をモノクロにするときも、それぞれのLumaの間引き方で結果が変わるので注意。

### ビット深度

 - `--bit-depth [8, 10, 12]`
   - 初期値：`8bit`

出力される画像のビット深度。

使えるビット深度とピクセルフォーマットとプロファイルの組み合わせに制限があるので注意。

### 色域

 - `--disable-full-color-range`（初期値）
 - `--enable-full-color-range`

例えば通常の8bitのYUVのフォーマットでは、Yの値として16-235、UとVの値として16-240しか使わないが、このフラグをenableにすると0-255のすべてを使うようになる。10/12ビットでも同様。デフォルトではfalse。

YUVからRGBへの変換方法を定めているRec.2020などを読んでも扱いが書いておらず、根拠がよくわからない。

## 速度と品質のトレードオフ

### エンコーダの用途

 - `--encoder-usage [good, realtime]`
   - 初期値：good（品質優先）

エンコーダのモードを指定する。goodの方が遅いが画質はよく、realtimeの方が速いが画質はおざなり。

### 利用スレッド数

 - `--threads <num-threads>`
   - 初期値：論理コア数（`nproc`コマンドで確認可能）

変換時に使うスレッド数を指定する。ただし実際にマルチスレッドされている気配がない。

### row based multi-threading of encoder

 - `--row-mt`
   - 初期値：なし（`row-mt`しない）

行単位でマルチスレッドでエンコードしてくれるようになるらしい。これを使うと`--threads`の意味が出てくるのかもしれない。ただし、画質に対して何かしらの影響があるかもしれない。

### cpu-used

 - `--cpu-used [0-8]`
   - 初期値：`1`

小さくすればするほど、CPUを犠牲にして画質を上げようとする。

## ビットレート制御

### チューニング・メトリクス

 - `--tune [ssim|psnr|cdef-dist|daala-dist]`
   - 初期値：`ssim` ([Structural Similarity](https://www.cns.nyu.edu/pub/lcv/wang03-preprint.pdf))

エンコーダが画質を最適するためにパラメータをチューニングするときに、どの指標をつかって画質を評価するか指定する。

[PSNRとSSIMは有名なので省略](https://dftalk.jp/?p=18111)。`cdef-dist`と`daala-dist`はv1.0.0の時点ではまだ実際には使えない様子（[#6](https://github.com/link-u/cavif/issues/6)）。

### レートコントロール・アルゴリズム

 - `--rate-control [q, cq]`
   - 初期値：`q`

出力される画像のファイルサイズの制御方法を指定する。

 - q: 品質を固定
 - cq: 品質を守りつつ、次で指定するbit rate限界まで品質を上げる（ただし努力目標？）。

### CQ Level

 - `--crf [0-63]`
   - 初期値：`32`

qとcqで守らせたい品質を指定する。値が低いほど画質はよい。

### ビットレート

 - `--bit-rate <kilo-bits per second>`
   - 初期値：`256 [kilo-bits per second]`

`--rate-control cq`で守らせるビットレート。1秒の動画という扱いにしているので、出力されるファイルはここで指定した`kilo-bits`を上回らない…はずだが、努力目標っぽい。

### qmax, qmin

 - `--qmax [0-63] (Maximum (Worst Quality) Quantizer)`
   - 初期値： `63`
 - `--qmin [0-63] (Minimum (Best Quality) Quantizer)`
   - 初期値： `0`

`--rate control[q, cq]`や`--crf [0-63]`で品質を固定した上で、さらに利用するq level（量子化レベル）の上限と下限を指定できる。ソースを読んだ限り、たぶんcrfよりさらにキツく上限と下限を制御するようになるんだと思うけれど、よくわからない。

### adaptive quantization

 - `--adaptive-quantization [none, variance, complexity, cyclic]`
   - 初期値：`none`

フレーム内で適応的に量子化パラメータを変える機能。デフォルトでnone。主観画質を上げるのに役立つらしい。

 - `--disable-adaptive-quantization-b`（初期値）
 - `--enable-adaptive-quantization-b`

さらにその進化版もあるらしい。違いはわからない。追加で`enable`にすることで有効になる。

### delta q / delta lf

 - `--delta-q [none, objective, perceptual]`
   - 初期値：`none`

スーパーブロックごとにqの値を変えることができる。デフォルトは`none`。`objective`にすると客観指標がよくなり、`perceptual`にすると主観的によくなるらしい。

#### Chroma Delta Q

 - `--disable-chroma-delta-q`（初期値）
 - `--enable-chroma-delta-q`

chromaでも有効にするかどうか

#### Delta LF

 - `--disable-delta-lf`（初期値）
 - `--enable-delta-lf`

Delta Qが有効になっているとDelta LoopFilterというのも有効にできる。

### quantisation matrices(qm) and quant matrix flatness

 - `--use-qm`
  - 初期値：無効。通常の`q`パラメータを使ったクオリティ制御を行う。
 - `--qm-min [0-15] (default: 5)`
 - `--qm-max [0-15] (default: 9)`
 - `--qm-min-y [0-15] (default: 10)`
 - `--qm-min-u [0-15] (default: 11)`
 - `--qm-min-v [0-15] (default: 12)`

上記のqとは別にQMatricesというのを使って品質を変えることも出来るらしい。qとは逆に、上がれば上がるほど品質が良いらしい。
デフォルトではoffで、`--use-qm`を指定して有効にした時だけ、他のオプションが意味を持つ。

### ロスレスモード

 - `--lossless`
   - 初期値：無効。lossyな圧縮をする

このフラグをつけると、ロスレスモードでエンコードする。アルファチャンネルをエンコードするときはこのほうがよいかもしれない。

なお、`--enable-full-color-range`を指定せずlimited rangeのYUVで変換する場合、RGBからYUVに変換する時点で情報が落ちる（変換が単射でない）ので完全に`lossless`にはならないので注意。

## Pre process

### モノクロ画像

 - `--monochrome`
   - 初期値：無効。色のある画像を出力する

モノクロで出力する。エンコーダが色差信号を無視する（モノクロにする）だけなので、入力画像はモノクロでなくてもよい。

### Sharpness

 - `--sharpness [0-7]`
   - 初期値： `0`（たぶん、sharpにしない）

たぶん、上げれば上げるほどシャープになる。ただしおすすめは0とのこと。

## Post process

### CDEF

 - `--disable-cdef`（初期値）
 - `--enable-cdef`

主観画質を上げるためのポストプロセス・フィルタである[CDEF](https://arxiv.org/abs/1602.05975)を有効にするかどうか決める。デコード時に適用され、無視できないぐらい重い。

### Loop Restoration Filter

 - `--disable-loop-restoration`（初期値）
 - `--enable-loop-restoration`

[失われてしまった高周波数領域を復活させるためのフィルタとのこと](https://www.spiedigitallibrary.org/conference-proceedings-of-spie/11137/1113718/AV1-In-loop-super-resolution-framework/10.1117/12.2534538.short?SSO=1)。

dav1dで試した限り結構負荷が高いので切ってもいいかもと思い、cavifではデフォルトでdisable。

## Coding parameter

### スーパーブロックサイズ

 - `--superblock-size [dynamic, 128, 64]`
   - デフォルト：`dynamic`

AV1では、画像をまずすべて同じ大きさのスーパーブロックに分割してから、その後それぞれのスーパーブロックを再帰的に分割して符号化していく。その大本のスーパーブロックのサイズを指定する。

デフォルトの`dynamic`を指定すると、短辺が480ピクセル以上の時`128x128`、それ以下のときは`64x64`のスーパーブロックで分割する。

### タイル分割

 - `--tile-rows [0-6]` `--tile-columns [0-6]`
   - 初期値：両方とも`0`

画像をそれぞれ `pow(2, <tile-rows>)`, `pow(2, <tile-columns>)`個の画像に分割して独立してエンコード・デコードする。

デフォルトではどちらも`0`で、分割せず１枚の画像として扱う。

### disable-(rect, ab, 1to4)-partitions

 - `--disable-rect-partitions`（初期値：`disable`にしない）
 - `--disable-ab-partitions`（初期値：`disable`にしない)
 - `--disable-1to4-partitions`（初期値：`disable`にしない)

ブロック分割する時にそれぞれの分割を無効にする。

rect/ab/1to4については次のAAを見よ：

```
//  Partition types.  R: Recursive
//
//  NONE          HORZ          VERT          SPLIT
//  +-------+     +-------+     +---+---+     +---+---+
//  |       |     |       |     |   |   |     | R | R |
//  |       |     +-------+     |   |   |     +---+---+
//  |       |     |       |     |   |   |     | R | R |
//  +-------+     +-------+     +---+---+     +---+---+
//
//  HORZ_A        HORZ_B        VERT_A        VERT_B
//  +---+---+     +-------+     +---+---+     +---+---+
//  |   |   |     |       |     |   |   |     |   |   |
//  +---+---+     +---+---+     +---+   |     |   +---+
//  |       |     |   |   |     |   |   |     |   |   |
//  +-------+     +---+---+     +---+---+     +---+---+
//
//  HORZ_4        VERT_4
//  +-----+       +-+-+-+
//  +-----+       | | | |
//  +-----+       | | | |
//  +-----+       +-+-+-+
```

### max/min partition size

 - `--min-partition-size [4|8|16|32|64|128]`
   - 初期値：`4`
 - `--max-partition-size [4|8|16|32|64|128]`
   - 初期値：`128`

上のパーティションの最小・最大サイズを指定する。デフォルトで最小は4、最大は128。

## Intra Edge filtering

 - `--enable-intra-edge-filter`（初期値）
 - `--disable-intra-edge-filter`

画像がスーパーブロックの定数倍でない限り、端っこにあまりの部分が出る。それらに対して掛けるフィルタを有効にするか否か。

デフォルトではenable。

### TX64

 - `--enable-tx64`（初期値）
 - `--disable-tx64`

64ピクセルのタイルでのTransformを許可するかしないか設定する。デフォルトではenable。

許可しない場合、64ピクセル以下のブロックになるまで必ず分割が走る。

### Flip IDTX

 - `--enable-flip-idtx`（初期値）
 - `--disable-flip-idtx`

AV1ではDCT以外にも[ADST](https://groups.google.com/a/webmproject.org/forum/#!topic/webm-discuss/JDxb0Qfzx7U)と呼ばれる上下左右非対称な基底を使った変換を行う事もあるし、そもそも変換を行わないこともある(IDTX; Identity TX)。

disableにすると、左右非対称な変換と恒等変換を無効にする。デフォルトはもちろん`enable`。

```
* This will enable or disable usage of flip and identity transform
* types in any direction. The default value is 1. Including:
* FLIPADST_DCT, DCT_FLIPADST, FLIPADST_FLIPADST, ADST_FLIPADST,
* FLIPADST_ADST, IDTX, V_DCT, H_DCT, V_ADST, H_ADST, V_FLIPADST,
* H_FLIPADST
 ```

同様に、`--use-dct-only` を指定するとDCTしか行わなくなる（初期値：`dct`以外も使う）。

`--use-default-tx-only` を指定すると、現在の予測モードから定まる「デフォルトのTX」以外は使わなくなる(`intra_mode_to_tx_type()`)。指定しない時（デフォルト）は`default-tx`以外も使う。

`--use-reduced-tx-set` を指定すると、16種類ある変換中、 `transforms w/o flip (4) + Identity (1)` の5種類しか使わなくなる(`av1_get_ext_tx_set_type()`)。指定しない時はこの5種類以外も使う。

### キーフレーム・フィルタリング

 - `--disable-keyfram-temporale-filtering`（初期値）
 - `--enable-keyframe-temporal-filtering`

フレーム同士の相関を見たりするフィルタをキーフレームにも掛けるかどうかを指定する。libaomではデフォルトでonになっているが、cavifでは静止画がターゲットなのでデフォルトでoffにしている。品質に問題があったらenableに戻してください。

### Intraフレーム各種

イントラフレームのフィルタリングを有効にするかどうか。たぶんoffにすると画質は下がる。ただ、デコーダ側で何かしらの処理は走ってたので、offにするとそのかわりデコードが速くなるかもしれない。

#### フィルタ

画質を上げるための各種フィルタ。

 - Filter Intra
   - `--enable-filter-intra`（初期値）
   - `--disable-filter-intra`
 - Smooth Intra
   - `--enable-smooth-intra`（初期値）
   - `--disable-smooth-intra`
 - Frame Superresolution
   - `--enable-superres`（初期値）
   - `--disable-superres`

#### 予測器

あるピクセル（複数でありうる）から他のピクセルの値を予測する。予測が当たる場合、圧縮率がよくなる。

##### [Paeth Intra](https://ieeexplore.ieee.org/document/8667544)

   - `--enable-paeth-intra`（初期値）
   - `--disable-paeth-intra`

##### Angle Delta

   - `--enable-angle-delta`（初期値）
   - `--disable-angle-delta`

##### CfL (Chroma prediction from Luma)

 - `--enable-chroma-from-luma`（初期値）
 - `--disable-chroma-from-luma`

Luma信号からChroma信号を予測する。

曰く「[どちゃくそ重いからHEVCではstrongly rejectedされたけど、現実的な範囲のものができたからAV1では有効にするぜ](https://arxiv.org/abs/1711.03951)」。デフォルトでon。

モノクロ画像ではどちらを指定しても関係ないかもしれないが、ゼロの値を使って「無」から何かを予測している可能性はあり、offにするとモノクロでもデコードが速くなる可能性は無いではない。

#### パレットモード

 - `--disable-palette`（初期値）
 - `--enable-palette`

有効にすると、8色しか使えないらしい。

現状では実際に有効にするには、さらに次の条件が守られていることが必要：

 - Superblockサイズが64でないと動かない（[av1_allow_palette](https://aomedia.googlesource.com/aom/+/refs/tags/v1.0.0-errata1-avif/av1/common/blockd.h#1113)）
 - 元画像の色数をカウントしてて、１ラインで使われている色の数が４色（実験的に仮で決めてる値っぽい）以下のラインが十分にないと動かない([set_screen_content_options](https://aomedia.googlesource.com/aom/+/refs/tags/v1.0.0-errata1-avif/av1/encoder/encoder.c#3857))

#### [Intra Block Copy](https://www.semanticscholar.org/paper/Intra-Block-Copy-in-HEVC-Screen-Content-Coding-Xu-Liu/5b8ef0e83b1e839a3ef62ab9821334247878444d/figure/0)

 - `--enable-intrabc`（初期値）
 - `--disable-intrabc`

同じ内容の領域があったらコピーするモードらしい。４コマ漫画で上のコマと下のコマでセリフ以外コピーしてる時とかは役に立つかもしれない。
