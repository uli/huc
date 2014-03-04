
 -*[ MAGIC KIT v2.51 ]*-
    -----------------

    Hi,


    PC-Engine news :
    ---------------

    This new release of MagicKit is mainly a HuC release,
    many new directives have been specialy added for the PC-Engine
    HuC C compiler, and several sub-routines were changed to work
    better in a C environment.

    The only new "asm" features are a few new sub-routines and
    macros to deal with scrolling and maps. The startup code
    as also been enhanced; especially the interrupt handlers,
    and gamepad support is also better.

    And to go with the new map sub-routines, INCBIN has been
    enhanced to support .FMP map files. Those files are produced
    by the excellent Mappy map editor :
    
    http://www.geocities.com/SiliconValley/Vista/7336/robmpy.htm


    NES news :
    ---------

    The NES version of the assembler also benefits of the .FMP
    map files support, but except that not much new things were
    added. There's just a new predefined function : SIZEOF().
    It returns the size of any data element.

    The NES version still lacks a good library, any help is
    welcome.


    Well I will let you code now. ;)

    And between two coding sessions, visit MagicKit homepage. :)

    http://www.magicengine.com/mkit/index.html


    Have fun!
    David


    Thanks to
    ---------

        - David Shadoff and Bt Garner for their input (and coding!) for
          improving the assembler, for providing some sample code, and
          for helping (a lot!) to write more documentation.

        - Zeograd for resuming my C compiler project and for
          continuing to work on it. Nice work Zeo!

        - Charles Doty for adding NES support to the assembler.

        - Tony Young for letting me distribute his NES 'JUNK' demo with
          MagicKit. Don't forget to ask him if you want to use his demo
          in one of your project or if you want to put it on your web
          page.

        - Jens Ch. Restemeier, and Paul Clifford, for making those
          excellent documentations on the PC-Engine hardware.

        - Hiroyuki Ito, for Japanese support - coming soon, a Japanese
          website and translated version of the documentation !

        - dil, for his Tetris CG artwork used in the SLIDSHOW demo.

        - J. H. Van Ornum, for his old 6502 assembler (May 11th, 1984).
          The source code of his assembler has been used as a base
          for MagicKit assembler.

        - DJ Delorie, for DJGPP.
          If you search for a fast, good and free C compiler,
          go to : <http://www.delorie.com/djgpp>

        - Charles Sandmann, for CWSDPMI.
          A pretty good DPMI server.
          <ftp://ftp.simtel.net/pub/simtelnet/gnu/djgpp/v2misc/csdpmi*.zip>

        - anybody who uses this assembler to write a demo or a game


--
David Michel <dmichel@easynet.fr>

