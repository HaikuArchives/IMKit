Compiling the code

First of all, a few notes: 

  * Consult the BuildNotes.txt in src/protocols/GoogleTalk, OSCAR, MSN, and Yahoo.

  * Consult the BuildNotes.txt in src/sample_clients/im_emoclient/

  * Running "make" will create a dir src/build where binaries are placed. Just so
    you know where to look for it.

  * When running "make symlinks" the Tracker add-ons will be symlink'ed 
    in ~/config/add-ons/Tracker where they belong, libim.so will be symlink'ed in
    ~/config/lib and im_server in ~/config/servers.

  * When running "make install" the Tracker add-ons will be copied 
    to ~/config/add-ons/Tracker where they belong, libim.so will be copied to
    ~/config/lib and im_server to ~/config/servers. The clients and tools etc
    will be copied to /boot/apps/im_kit.


Now, on to the making:

  To compile do this 
     0. Make sure you've read the protocol readmes and followed the instructions!
     1. Open a Terminal 
     2. Change to the imkit/src folder 
     3. Run "make"
     4. Run "make install" or "make symlinks"

Simple enough.
