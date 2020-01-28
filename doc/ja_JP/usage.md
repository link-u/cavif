# コマンドラインオプションの説明

選択肢から選ぶものは、断りがない限り一番最初がデフォルト。

## 入出力

### 入力

`-i input.png`

 - 対応フォーマット
   - 1/2/4/8bit Gray
   - 8/16bit RGB
   - alpha channel対応（ただし現状無視される）
### 出力

`-o output.avif`

## メタデータ

### 切り抜き・回転・反転

適用される順番は「切り抜き→回転→反転」

#### 切り抜き

`--crop-size widthN/widthD,heightN/heightD`  
`--crop-offset horizOffN/horizOffD,vertOffN/vertOffD`

表示時に切り抜くサイズとオフセットを分数で指定する。

デフォルトでは指定されたサイズで中心から切り抜き、offsetが指定されたら、その分移動したところから切り抜く。

例：`--offset-size 1000/3,1000/7`（333.3x142.9で中心から切り抜く）

#### 回転

`--rotation [0, 90, 180, 270]`

表示時に回転する。反時計回り。

#### 反転

`--mirror [vertical, horizontal]`

表示時に反転する。

## AV1 シーケンシャルヘッダ

### 静止画用の削減されたヘッダを出力せず、動画用のフルのヘッダを出力する

`--full-still-picture-header`

このオプションを指定すると、動画用のヘッダを出力する。静止画専用のヘッダにくらべて、3バイトぐらい長くなる。

## プロファイル

### AV1 シーケンス・プロファイル

`--profile (0=base, 1=high, 2=professional)`

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

`--pix-fmt [yuv420, yuv422, yuv444]`

出力される画像のピクセルフォーマット。

使えるビット深度とピクセルフォーマットとプロファイルの組み合わせに制限があるので注意。カラー画像をモノクロにするときも、それぞれのLumaの間引き方で結果が変わるので注意。

### ビット深度

`--bit-depth [8, 10, 12]`

出力される画像のビット深度。

使えるビット深度とピクセルフォーマットとプロファイルの組み合わせに制限があるので注意。

### 色域

`--disable-full-color-range`  
`--enable-full-color-range`

例えば通常の8bitのYUVのフォーマットでは、Yの値として16-235、UとVの値として16-240しか使わないが、このフラグをenableにすると0-255のすべてを使うようになる。10/12ビットでも同様。デフォルトではfalse。

YUVからRGBへの変換方法を定めているRec.2020などを読んでも扱いが書いておらず、根拠がよくわからない。

## 速度と品質のトレードオフ

### エンコーダの用途

`--encoder-usage [good, realtime]`

エンコーダのモードを指定する。goodの方が遅いが画質はよく、realtimeの方が速いが画質はおざなり。

### 利用スレッド数

`--threads <num-threads>`

変換時に使うスレッド数を指定する。何も指定しないと論理コア数。ただし実際にマルチスレッドされている気配がない。

### row based multi-threading of encoder

`--row-mt`

行単位でマルチスレッドでエンコードしてくれるようになるらしい。これを使うと`--threads`の意味が出てくるのかもしれない。ただし、画質に対して何かしらの影響があるかもしれない。デフォルトはオフ。

### cpu-used

`--cpu-used [0-8]`

小さくすればするほど、CPUを犠牲にして画質を上げようとする。
デフォルトは1。

## ビットレート制御

### チューニング・メトリクス

`--tune [ssim|psnr|cdef-dist|daala-dist]`

エンコーダが画質を最適するためにパラメータをチューニングするときに、どの指標をつかって画質を評価するか指定する。

PSNRとSSIMは有名なので省略。CDEF-distはよくわからない。daalaはAV1の前身だが、daala-distが何なのかはよくわからない。

少し試した限りでは正直よくわからなかった。結果はその目で確かめてください。デフォルトはSSIM。

### レートコントロール・アルゴリズム

`--rate-control [q, cq]`

出力される画像のファイルサイズの制御方法を指定する。

 - q: 品質を固定
 - cq: 品質を守りつつ、次で指定するbit rate限界まで品質を上げる（ただし努力目標？）。

### CQ Level

`--crf [0-63]`

qとcqで守らせたい品質を指定する。値が低いほど画質はよい。

### ビットレート

`--bit-rate <kilo-bits per second>`

`--rate-control cq`で守らせるビットレート。1秒の動画という扱いにしているので、出力されるファイルはここで指定した`kilo-bits`を上回らない…はずだが、努力目標っぽい。

### qmax, qmin

`--qmax [0-63] (Maximum (Worst Quality) Quantizer)`  
`--qmin [0-63] (Minimum (Best Quality) Quantizer)`

`--rate control[q, cq]`や`--crf [0-63]`で品質を固定した上で、さらに利用するq level（量子化レベル）の上限と下限を指定できる。ソースを読んだ限り、たぶんcrfよりさらにキツく上限と下限を制御するようになるんだと思うけれど、よくわからない。

### adaptive quantization

`--adaptive-quantization [none, variance, complexity, cyclic]`

フレーム内で適応的に量子化パラメータを変える機能。デフォルトでnone。主観画質を上げるのに役立つらしい。

`--disable-adaptive-quantization-b`  
`--enable-adaptive-quantization-b`

さらにその進化版もあるらしい。違いはわからない。

### delta q / delta lf

`--delta-q [none, objective, perceptual]`

スーパーブロックごとにqの値を変えることができる。デフォルトではnone。objectiveにすると客観指標がよくなり、perceptualにすると主観的によくなるらしい。

#### Chroma Delta Q

`--disable-chroma-delta-q`  
`--enable-chroma-delta-q`

chromaでも有効にするかどうか

#### Delta LF

`--disable-delta-lf`  
`--enable-delta-lf`

Delta Qが有効になっているとDelta LoopFilterというのも有効にできる。デフォルトではdisable。

### quantisation matrices(qm) and quant matrix flatness

`--use-qm`  
`--qm-min [0-15] (default: 5)`  
`--qm-max [0-15] (default: 9)`  
`--qm-min-y [0-15] (default: 10)`  
`--qm-min-u [0-15] (default: 11)`  
`--qm-min-v [0-15] (default: 12)`

上記のqとは別にQMatricesというのを使って品質を変えることも出来るらしい。qと同じく、上がれば上がるほど品質が良いらしい。
デフォルトではoffで、`--use-qm`を指定して有効にした時だけ、他のオプションが意味を持つ。

### ロスレスモード

`--lossless`

このフラグをつけると、ロスレスモードでエンコードする。アルファチャンネルをエンコードするときはこのほうがよいのでは？

## Pre process

### モノクロ画像

`--monochrome`

モノクロで出力する。エンコーダが色差信号を無視する（モノクロにする）だけなので、入力画像はモノクロでなくてもよい。

### Sharpness

`--sharpness [0-7] (default = 0)`

たぶん、上げれば上げるほどシャープになる。ただしおすすめは0とのこと。

## Post process

### CDEF

`--disable-cdef`
`--enable-cdef`

主観画質を上げるためのポストプロセス・フィルタである[CDEF](https://arxiv.org/abs/1602.05975)を有効にするかどうか決める。デコード時に適用され、無視できないぐらい重い。

デフォルトでdisable。

### Loop Restoration Filter

`--disable-loop-restoration`  
`--enable-loop-restoration`

[失われてしまった高周波数領域を復活させるためのフィルタとのこと](https://www.spiedigitallibrary.org/conference-proceedings-of-spie/11137/1113718/AV1-In-loop-super-resolution-framework/10.1117/12.2534538.short?SSO=1)。

dav1dで試した限り結構負荷が高いので切ってもいいかもと思い、cavifではデフォルトでdisable。

## Coding parameter

### スーパーブロックサイズ

`--superblock-size [dynamic, 128, 64]`

AV1では、画像をまずすべて同じ大きさのスーパーブロックに分割してから、その後それぞれのスーパーブロックを再帰的に分割して符号化していく。その大本のスーパーブロックのサイズを指定する。

デフォルトの`dynamic`を指定すると、短辺が480ピクセル以上の時`128x128`、それ以下のときは`64x64`のスーパーブロックで分割する。

### タイル分割

`--tile-rows [0-6]` `--tile-colums [0-6]`

画像をそれぞれ `pow(2, <tile-rows>)`, `pow(2, <tile-colums>)`個の画像に分割して独立してエンコード・デコードする。

デフォルトではどちらも0で、分割せず１枚の画像として扱う。

### disable-(rect, ab, 1to4)-partitions

`--disable-rect-partitions`  
`--disable-ab-partitions`  
`--disable-1to4-partitions`

ブロック分割する時にそれぞれの分割を無効にする。デフォルトでは全部有効。

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

`--min-partition-size [4|8|16|32|64|128]`  
`--max-partition-size [4|8|16|32|64|128]`

上のパーティションの最小・最大サイズを指定する。デフォルトで最小は4、最大は128。

## Intra Edge filtering

`--enable-intra-edge-filter`  
`--disable-intra-edge-filter`

画像がスーパーブロックの定数倍でない限り、端っこにあまりの部分が出る。それらに対して掛けるフィルタを有効にするか否か。

デフォルトではenable。

### TX64

`--enable-tx64`  
`--disable-tx64`

64ピクセルのタイルでのTransformを許可するかしないか設定する。デフォルトではenable。

許可しない場合、64ピクセル以下のブロックになるまで必ず分割が走る。

### Flip IDTX

`--enable-flip-idtx`  
`--disable-flip-idtx`

AV1ではDCT以外にも[ADST](https://groups.google.com/a/webmproject.org/forum/#!topic/webm-discuss/JDxb0Qfzx7U)と呼ばれる上下左右非対称な基底を使った変換を行う事もあるし、そもそも変換を行わないこともある(IDTX; Identity TX)。

disableにすると、左右非対称な変換と恒等変換を無効にする。デフォルトはもちろんenable。

```
* This will enable or disable usage of flip and identity transform
* types in any direction. The default value is 1. Including:
* FLIPADST_DCT, DCT_FLIPADST, FLIPADST_FLIPADST, ADST_FLIPADST,
* FLIPADST_ADST, IDTX, V_DCT, H_DCT, V_ADST, H_ADST, V_FLIPADST,
* H_FLIPADST
 ```

同様に、`--use-dct-only` を指定するとDCTしか行わなくなる。

`--use-default-tx-only` を指定すると、現在の予測モードから定まる「デフォルトのTX」以外は使わなくなる(`intra_mode_to_tx_type()`)。

`--use-reduced-tx-set` を指定すると、16種類ある変換中、 `transforms w/o flip (4) + Identity (1)` の5種類しか使わなくなる(`av1_get_ext_tx_set_type()`)。

### キーフレーム・フィルタリング

`--disable-keyfram-temporale-filtering`  
`--enable-keyframe-temporal-filtering`

フレーム同士の相関を見たりするフィルタをキーフレームにも掛けるかどうかを指定する。libaomではデフォルトでonになっているが、cavifでは静止画がターゲットなのでデフォルトでoffにしている。品質に問題があったらenableに戻してください。

### Intraフレーム各種

イントラフレームのフィルタリングを有効にするかどうか。たぶんoffにすると画質は下がる。ただ、デコーダ側で何かしらの処理は走ってたので、offにするとそのかわりデコードが速くなるかもしれない。

#### フィルタ

`--enable-filter-intra`  
`--disable-filter-intra`

`--enable-smooth-intra`  
`--disable-smooth-intra`

`--enable-superres`  
`--disable-superres`

以上全部デフォルトでon。

#### 予測器

`--enable-paeth-intra`  
`--disable-paeth-intra`

`--enable-angle-delta`  
`--disable-angle-delta`

以上全部デフォルトでon。

#### CfL (Chroma prediction from Luma)

`--enable-chroma-from-luma`  
`--disable-chroma-from-luma`

Luma信号からChroma信号を予測する。当たる場合、圧縮率がよくなる。

曰く「[どちゃくそ重いからHEVCではstrongly rejectedされたけど、現実的な範囲のものができたからAV1では有効にするぜ](https://arxiv.org/abs/1711.03951)」。デフォルトでon。

モノクロ画像ではどちらを指定しても関係ないかもしれないが、ゼロの値を使って「無」から何かを予測している可能性はあり、offにするとモノクロでもデコードが速くなる可能性は無いではない。

#### パレットモード

`--disable-palette`  
`--enable-palette`

有効にすると、8色しか使えないらしい。デフォルトはoff。

#### [Intra Block Copy](https://www.semanticscholar.org/paper/Intra-Block-Copy-in-HEVC-Screen-Content-Coding-Xu-Liu/5b8ef0e83b1e839a3ef62ab9821334247878444d/figure/0)

`--enable-intrabc`  
`--disable-intrabc`

同じ内容の領域があったらコピーするモードらしい。４コマ漫画で上のコマと下のコマでセリフ以外コピーしてる時とかは役に立つかもしれない。デフォルトはon。
