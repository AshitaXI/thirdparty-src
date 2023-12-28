<div align="center">
    <img width="128" src="https://github.com/AshitaXI/Ashita/raw/master/repo/ashita.png" alt="ashita">
    </br>
</div>

<div align="center">
    <a href="https://discord.gg/Ashita"><img src="https://img.shields.io/discord/264673946257850368.svg?style=for-the-badge" alt="Ashita Discord Server" /></a>
    <a href="LICENSE.md"><img src="https://img.shields.io/badge/License-GPL_v3-blue?style=for-the-badge" alt="license" /></a>
    <a href="https://ashitaxi.com/"><img src="https://img.shields.io/badge/Homepage-link-blue?style=for-the-badge" alt="link" /></a>
    <a href="https://docs.ashitaxi.com/"><img src="https://img.shields.io/badge/Documentation-link-blue?style=for-the-badge" alt="link" /></a>
</div>

# Thirdparty (Source Code)

This repository contains the source code to the Ashita v4 plugin, **thirdparty**.

**thirdparty** is a plugin for Ashita v4 that enables third-party external applications to communicate with Ashita and the game client. This plugin implements three interfaces that are designed around the way the `WindowerHelper` library is constructed, allowing applications designed for Windower (and Ashita) to work with Ashita.

The three interfaces this implements are:

  - **Console** - _Allows external applications to send commands to Ashita._
  - **Keyboard** - _Allows external applications to send keyboard input to Ashita._
  - **Text** - _Allows external applications to send font/text object settings to Ashita._

This plugin implements the expected interfaces used by: [https://github.com/Windower/Helper](https://github.com/Windower/Helper)

## License

**thirdparty** is licensed under [GNU GPL v3](LICENSE.md).
