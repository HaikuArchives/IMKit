IM Kit - ReadMe
===============

IM Kit is a kit for Haiku and Zeta (BeOS R5 should be supported to, but we don't
have developers and testers available) that connects to instant messaging networks
such as Jabber, GoogleTalk, MSN, ICQ, AIM and Yahoo and creates a connection between
your contacts and People files.

== Building the code ==

IM Kit uses a Jam-based build system.
All you need to do is:

$ ./configure
$ jam -q

And then, to install:

$ jam -q install
