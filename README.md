# STEP Week6 Homework

malloc challenge

<br>

ほぼ全部メモみたいなものになってる、、すみませんでした

#### 自分用メモ

- 8 <= |size| <=4000  is a multiple of 8 bytes（なぜ4096ではないの？

- **mmap_from_system() / munmap_to_system()**だけが使用可能

- デフォルト結果

  - 10,70 | 6,40 | 93,7 | 17472,15 | 12332,15

- データ構造

  - my_metadata_t: 単方向連結リスト
  - my_heap_t: headを記録する（？？？）

- functionと変数

  - my_malloc(size)
    - metadata: 空き領域を指すポインター

- sizeof(my_metadata_t) = 16

<br>

### 1. First Fit -> Best Fit

#### 方針&メモ

- while metadata:ループで全ての空き領域をアクセスして、metadata.size > sizeのmetadata.sizeの中で一番小さいmetadataのアドレスから長さsizeの領域を確保

- 一番前のmetadataの位置: my_heap.free_headに記録される

- 実質連結リスト任意のノードを削除する問題になる（それより簡単）

  <br>

#### 結果

- 979,70 | 641,40 | 755,50 | 7237,71 | 4020,74
- hikaliumさんと同じです！
- ![](https://github.com/xueyi-2021/STEP_Week6/blob/master/best_fit_malloc.png)

<br>

### 2. Free List Bin

#### 方針

- sizeのサイズは0~4096なので適当に分ける
  - 0\~1024, 1025\~2048, 2049\~3072, 3073\~4096のように4つ分ける
  - と思って4つのmy_heapを定義したい時コメント見た↓↓↓
    - (DO NOT ADD ANOTHER STATIC VARIABLES!)
    - これはどういう意味だろう…わからないのでとりあえず4つ定義した



#### デバッグ

- my_heap_t my_bins[4];でmake killedになった
- my_heap_t my_bins[1];は実行できたが、どこが問題点なの…
- とりあえず2つのbinにしてみたいと思う
  - またkilled......
  - 新しいエラー：An allocated object is broken!make: *** [run] Abort trap: 6
  - バグの原因が判明、多分初期化の問題、実行はとりあえずできた
- 瓶を増やす！



#### 方針の続き

- 4096は2の12乗なので(12-3+1)=10個瓶を作る(size>=8なので8以下の瓶作る必要がないと思う)
- 瓶を探すっていう操作はmy_mallocとmy_add_to_free_listの中で2回実行する必要があるので、find_bin関数を定義する（モジュール化）



#### 結果

- 837,70 | 641,40 | 205,50 | 313,71 | 274,74
- 時間だいぶ短くなった！！（チャレンジ4と5だけ…）
  - 一つの瓶の場合では特定な入力パターンに対して早いかも、複数の瓶の場合は一般的に早くなる(汎用性が高い)っていう感じ
- 実質加速化のbest-fitなのでUtilizationは変化しない
- ![](https://github.com/xueyi-2021/STEP_Week6/blob/master/free_list_bin_malloc.png)

<br>

### 3. 空き領域の結合(merge)

#### 方針（ほぼメモ）

- my_add_to_free_listを改造する（？？）
- 左と右を探して、もしあれば結合して対応するbinに入れる、か
- metadata多分アドレス順で連結するとは限らない
  - 10個の瓶の中で全てのノード一度訪問するのか
- 隣接するかどうかの判定