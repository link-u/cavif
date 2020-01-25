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

表示時に回転する。半時計回り。

#### 反転

`--mirror [vertical, horizontal]`

表示時に反転する

## エンコード・クオリティ

### AV1 Profile

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

### モノクロ画像

`--monochrome`

モノクロで出力する。エンコーダがモノクロにするので、入力画像はモノクロでなくてもよい。

### エンコーダの用途

`--encoder-usage [good, realtime]`

エンコーダのモードを指定する。goodの方が遅いが画質はよく、realtimeの方が速いが画質はおざなり。

### 利用スレッド数

`--threads <num-threads>`

変換時に使うスレッド数を指定する。何も指定しないと理論コア数。ただし実際にマルチスレッドされている気配がない。

### ピクセルフォーマット

`--pix-fmt [yuv420, yuv422, yuv444]`

出力される画像のピクセルフォーマット。

使えるビット深度とピクセルフォーマットとプロファイルの組み合わせに制限があるので注意。カラー画像をモノクロにするときも、それぞれのLumaの間引き方で結果が変わるので注意。

### ビット深度

`--bit-depth [8, 10, 12]`

出力される画像のビット深度。

使えるビット深度とピクセルフォーマットとプロファイルの組み合わせに制限があるので注意。

### レートコントロール

`--rate-control [q, cq]`

出力される画像のサイズを指定する。

 - q: 品質を固定
 - cq: 次で指定するbit rateを守りつつ、品質を上げる

### ビットレート

`--bit-rate <kilo-bits per second>`

`--rate-control cq`で守らせるビットレート。1秒の動画という扱いにしているので、出力されるファイルはここで指定した`kilo-bits`を上回らない…はずだが、努力目標っぽい。

### 静止画ヘッダー

`--full-still-picture-header`

静止画前提のAV1のシーケンスヘッダーが出力されることになっている（出力されなかった）。もし意図通りに動けば、3バイトぐらいヘッダが短くなる。

### 色域

`--disable-full-color-range`
`--enable-full-color-range`

例えば通常の8bitのYUVのフォーマットでは、Yの値として16-235、UとVの値として16-240しか使わないが、このフラグをenableにすると0-255のすべてを使うようになる。デフォルトではfalse。

YUVからRGBへの変換方法を定めているRec.2020などを読んでも扱いが書いておらず、根拠がよくわからない。
