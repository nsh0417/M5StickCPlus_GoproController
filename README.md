# M5StickCPlus_GoproController

以下を参考に後々書いてみる
- https://cpp-learning.com/readme/
- https://deeeet.com/writing/2014/07/31/readme/

## Overview

GoProをM5StickC+から録画／停止を操作可能とするアプリです。

## Description

GoPro(**現時点ではHERO+のみ**)に対して、M5StickC+は以下の機能を持つ。

1. 録画開始機能
2. 録画停止機能
3. 録画中監視機能
4. 録画再開機能（監視機能にて、意図しない録画停止を確認した場合）

## Demo

![全体構成イメージ](./images/M5StickCPlus_GoproController_image.png)

## Requirement

- 製品
  - [M5StickC+](https://www.switch-science.com/catalog/6470/)
  - [GoPro](https://gopro.com/ja/jp/)
- 開発環境
  - Windows 10 Home 64bit
  - [Visual Studio Code](https://azure.microsoft.com/ja-jp/products/visual-studio-code/)
  - [PlatformIO IDE](https://platformio.org/platformio-ide)

## Usage

## Install

## TODO

- メッセージが古いものから消えるように対応する
- 画面表示をアイコンとか使って表示する
- コメントの書き方とかが適当なので修正する
- リファクタリング（とりあえず動くことが最優先）
- GOPROのSSIDやパスは設定ファイルで対応できるようにする

## Licence

[MIT](https://github.com/tcnksm/tool/blob/master/LICENCE)

## Author

[nsh0417](https://github.com/nsh0417)

## References

- [goprowifihack](https://github.com/KonradIT/goprowifihack/)
  - [WifiCommands](https://github.com/KonradIT/goprowifihack/blob/master/HERO/WifiCommands.md)
- [GoPro をライブカメラにしてターミナルから操作する](https://android.benigumo.com/20180116/gopro-hero6-wifi/)
- [ＧｏＰｒｏ　ＨＥＲＯ４を買いました。](http://hiroeki1.blog129.fc2.com/blog-entry-400.html)
- [M5StickCであそぶ 〜ボタンとLEDを使う〜](https://make-muda.net/2019/09/6906/)
- [M5StickCであそぶ 〜バッテリーを使う〜](https://make-muda.net/2019/09/6946/)
- [M5StickC+](https://github.com/m5stack/M5StickC-Plus)
- [M5StackでWi-Fi通信](https://msr-r.net/m5stack-wifi/)
- [電源管理(AXP192)](https://lang-ship.com/reference/unofficial/M5StickC/Class/AXP192/)
