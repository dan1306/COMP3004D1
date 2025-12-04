# HinLIBS Deliverable 2

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Setup and Run Instructions
------------------------------------------------------------------------------------------------------------------------------------------------------------------------

1) Unzip  team_34_D2.zip
2) Open Qt Creator in COMP-3004 VM, in Oracle VM Virtualbox
3) Open the application Qt Creator
4) In Qt Creator assuming you have import the unzipped folder of this assignment into the shared folder between your main PC and the Virtual Box, in Project section of Qt Creator click Open as opposed to New.
5) From there find and open the unzipped assignment folder and select and open hinlibs.pro
6) After that Configure, Build, and Run the program.

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# User Accounts
------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Use these usernames as shown (case sensitive)
Patron:
1) Alice
2) Bob
3) Carmen
4) Dinesh
5) Eve

Librarian:
1) Belinda

Admin:
1) Admin
 
To ensure the application is turnkey and runs as seamlessly as possible, without any manual configuration, the hinlibs.pro file includes an instruction to copy the SQLite database from the project’s db/ folder into the build directory when the project is compiled. This ensures tha    the app always knows where to find the database file on launch, allowing anyone to open, build, and run the program with as few steps as possible. This prevents common errors like “unable to open database file,” which occur when Qt runs the executable from the build directory and the database isn’t present there. 

All data is fully persistent, data does NOT reset when the application is closed.

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Seed data loaded at startup
------------------------------------------------------------------------------------------------------------------------------------------------------------------------

1) 7 users (5 patrons, 1 admin, 1 librarian)
2) 20 items in catalogue (5 fiction book, 5 non-fiction book, 3 magazines, 3 movies, 4 video games)

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Application Features
------------------------------------------------------------------------------------------------------------------------------------------------------------------------

1) Patron Features:
    - Browse catalogue
    - Borrow items
    - Return items
    - Place holds
    - Cancel holds
    - View account (loans & holds)

2) Librarian Features:
    - Add items to the catalogue (AddItemDialog)
    - Remove items from the catalogue
    - Search for patrons
    - View patron loans
    - Process returns on behalf of patrons
    - Refresh and inspect catalogue contents


