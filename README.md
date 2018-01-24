# gc-3ds-input-redirection
This tool gets input from the official Wii U GC Adapter through the [wii-u-gc-adapter](https://github.com/ToadKing/wii-u-gc-adapter) and sends it to a 3DS running [Luma3DS](https://github.com/AuroraWright/Luma3DS) CFW.

## Usage for geeks
The following is aimed to people who want to use the software right now and have general knowledge about Linux OSs. If you don't want to lose time on technical stuff and just want to quickly get everything working, an all-in-one installation script will be released soon.

### Dependencies
This program depends on `SDL2`. Install it using your package manager of choice or clone and compile it from source.

    # Example on Debian/Ubuntu/Derivatives
    $ sudo apt-get install libsdl2-dev

### Installation
Clone the repository wherever you want and then run the following commands:

    $ cd gc-3ds-input-redirection
    $ cmake .
    $ make

### Usage
1. Create a file named `g3ir.list` that will contain the IP addresses of the 3DS's you want to connect to (one per line), and place it in the same directory as the executable.
Example:

    192.168.1.14
    192.168.1.20

2. Start the [wii-u-gc-adapter](https://github.com/ToadKing/wii-u-gc-adapter) executable as root on a terminal or in the background (detailed instructions on the project page).
3. Start gc-3ds-input-redirection executable on a second terminal (a standard user should be enough).
4. On the 3DS(s) open up the Rosalina menu by pressing `L+Down+Select` and select `Miscellaneous options... -> Start InputRedirection`.
5. Enjoy.

## Usage for non-geeks
An installation script will be released soon including many more features. Just keep waiting...

## Credits
Many thanks to [contiser](https://github.com/contiser) for helping me setting up some server logic. That's going to be used in next releases. More details to come.