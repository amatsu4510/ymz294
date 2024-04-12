# ymz294

[試作品(Youtube)](https://www.youtube.com/watch?v=9vhfXz8pEZI)
ブレッドボードに実装して、ダイソーのスピーカーつないで音鳴らしてみたやつ

## 概要
YMZについて
YMZ294はYMZ2149とソフトウエアコンパチブルの音源LSIで、3系列の矩形波発生器と1系列のノイズ発生器、エンベロープ発生器を内蔵しており、メロディ音効果音の発音が可能です。

### 特長
- 矩形波3音+ノイズ1音の音源
- 5ビットDACを3個内蔵し、3音ミキシング出力
- CS、WR制御信号と8ピットデータバスによる汎用CPUインターフェース
- 8オクターブの広い発音域
- エンベロープ発生器による滑らかな減衰感
- 5V単一電源

秋月電子から引用

## ソフト仕様
1. YMZ294を使用して音楽が再生できる
2. 再生・停止・曲送り・曲戻しができる
3. 最大発音数9和音
4. SDカードに書き込まれたSMF(スタンダードMIDIファイル)を解析できる

## 主な使用部品
- マイコン(Arduino Nano Every)
- 音源IC(YMZ294)
- microSDカード

## 回路図 ※参考程度に・・・
![](https://github.com/amatsu4510/ymz294/blob/images/%E5%9B%9E%E8%B7%AF%E5%9B%B3.png)

## 参考ページ
### YMZ294
- [YMZ294(秋月電子)](https://akizukidenshi.com/catalog/g/g112141/).
- [YMZ294使い方](https://blog-e.uosoft.net/entry/20160703/1467476171).
### MIDI
- [MIDI 1.0 規格書](https://amei.or.jp/midistandardcommittee/MIDI1.0.pdf).
- [SMF (Standard MIDI Files) の構造](https://sites.google.com/site/yyagisite/material/smfspec).
### SDカード
- [SDカードライブラリ(Arduino)](https://garretlab.web.fc2.com/arduino.cc/www/reference/ja/libraries/sd/)

