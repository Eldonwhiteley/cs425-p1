# Project 1 - Complex ;) SMTP Client

- Name: Eldon Whiteley
- Email: eldonwhiteley@u.boisestate.edu
- Class: 434-001

## Known Bugs or Issues

As of now I am not aware of any bugs. Many of my tests run through good and as many bad paths as I thought
necessary to ensure the functions work properly. I skipped tests for the thin wrapper functions as they
would be difficult to force failures on.

## Experience

The experience was quite interesting, I learned a lot about SMTP and simple network code. I relied on
man pages and copied quite a few examples from them to set up the network, connect, create sockets, send
and read data, and so on. It took quite some time to implement the bigger framework around the read and
write functions.  
Using callback functions was new to me as well, it was an interesting way to design for testing and I
probably could have implemented it in a cleaner way. Maybe I could use a static variable to store the 
socket and other relevant information, or represent the whole session as a struct and get each member
by just using `session.mail_from`, `session.socket`, `session->body`. Since it works now I don't plan
on changing it, but just some thoughts.