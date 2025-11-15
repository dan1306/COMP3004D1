# HinLIBS Deliverable 1

# Setup and Run Instructions

1. Unzip team-34-D1.zip and place it in your shared folder that links the COMP-3004 VM and your PC.
2. Open Qt Creator inside COMP-3004 VM, Oracle VirtualBox image.
3. In Qt Creator, (assuming the unzipped assignment folder is available through the shared folder between your main PC and the VirtualBox VM) go to the Projects section and click Open (do not choose “New Project”).
4. From there find the unzipped assignment folder, in your shared folder. Look through and select the file HinLIBS.pro in the folder HinLIBS.
5. After that it should Configure, then click Build, and then Run.

# User Accounts For Running the Program

Patron:

1. Alice
2. Bob
3. Carmen
4. Dinesh
5. Eve

Librarian:

1. Librarian

Admin:

1. Admin

Only Patron functionality is implemented for Deliverable 1.

# Seed data loaded at startup

1. 7 users (5 patrons, 1 admin, 1 librarian)
2. 20 items in catalogue (5 fiction book, 5 non-fiction book, 3 magazines, 3 movies, 4 video games)

All items start with Available status on launch.

# Notes
1. Data is in-memory only for  D1, no databases for persistent storage.
2. Upon exiting the application, all data is reset, this includes every interaction users have with the Library system.


