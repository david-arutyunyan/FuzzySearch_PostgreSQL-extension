# Fuzzy Search Mechanism (PostgreSQL extension)

This extension is installed in the same way as any other PostgreSQL extension written in C.
Before installation, make sure that the PostgreSQL server is installed on your computer.

1. **Build:**
    * You need to open a command prompt and move to the directory where the extension files are located. In this case, you can clone the repository and move to the resulting folder.
    * Then you need to install the libpq-dev library. You can do this using the following command: `sudo apt-get install libpq-dev`.
    * Next, you need to run `make` and then `make install`. It is worth remembering that administrator rights may be required, so it is better to execute these commandsing sudo. Now you can proceed to installing the extension itself in PostgreSQL.

2. **Creating:**
    * You need to log in to PostgreSQL as a superuser. To log in to PostgreSQL as a superuser, you can use the `psql` command line tool with a postgres user account.
    * Next, you need to create the extension itself using the `CREATE EXTENSION "fuzzy";` command. If an error appears stating that this extension is already installed, then you must first execute the `DROP EXTENSION "fuzzy";` command.
