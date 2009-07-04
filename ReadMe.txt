IM Kit - ReadMe
===============

IM Kit is a kit for Haiku and Zeta (BeOS R5 is no longer supported) that connects to
instant messaging networks such as Jabber, GoogleTalk, MSN, ICQ, AIM and Yahoo and
creates a connection between your contacts and People files.

== Building the code ==

IM Kit uses a Jam-based build system.
All you need to do is:

$ ./configure
$ jam -q

And then, to install:

$ jam -q install

If you want to uninstall:

$ jam -q uninstall

To build a .zip archive:

$ jam -q imkit

Triggering the post-install script:

$ jam post-install
