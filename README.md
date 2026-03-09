# Ride-Sharing 1.0 (VITC)

## Overview
This is a simple, console-based ride-sharing application built in C, specifically conceptualized for the VITC (Vellore Institute of Technology Chennai) campus environment. The application allows users to search for currently available rides to or from the campus, view all available rides, and look up the historical travel destinations of users.

## Features
- **Show All Available Rides:** Lists all standard active rides from the database.
- **Find Ride:** Searches for a specific ride based on:
  - Direction (leaving VITC or returning to VITC)
  - Date (e.g., DD-MM-YYYY)
  - Destination/Source Location
- **Show User History:** Displays a list of all known users and their past travel destinations.

## How It Works
The app operates completely in memory during runtime and uses CSV (Comma Separated Values) files as its pseudo-database. 

### Data Structures & Memory Management
The core application relies on two main structures:
- `Ride`: Stores the details of an active ride (Name, Date, Source, Destination, and Departure Time).
- `UserHistory`: Tracks a user's name and all the destinations they have traveled to in the past.

The application dynamically allocates memory (`malloc` and `realloc`) while reading the CSV files. This allows it to efficiently scale and handle any number of users, rides, and destinations without memory hard limits.

### Application Flow
1. **Startup Initialization:** When `Main.exe` is run, it attempts to load two CSV files (`rides.csv` and `rides_current.csv`).
2. **Data Parsing:** As it reads the files, it sanitizes inputs (removing trailing spaces/quotes), standardizes date formats, and splits composite strings like "Tambaram to VITC" into discrete origin and destination locations.
3. **Interactive Menu:** The application enters a `while(1)` loop, presenting an interactive CLI menu to the user.
4. **Clean Exit:** Upon selecting the 'Exit' option, the application safely frees all dynamically allocated memory to prevent memory leaks and terminates.

## Prerequisites and Setup
To run this application, you need the compiled executable and the accompanying data files.

### ⚠️ Important Note Regarding Data Files
This application **requires** two specific CSV files to function correctly. These files are **NOT included** in this repository due to privacy constraints, as they contain personal user data and ride histories.

1. `rides.csv` - Contains historical ride records used to build user travel profiles.
2. `rides_current.csv` - Contains the active, currently available rides.

**If these files are missing, the application will fail to start with a "Could not open" error.**

To run the application, you must provide your own mock CSV files or ensure the original files are placed in the identical root directory as `Main.exe`.

### Compilation
If you need to recompile the source code:
```bash
gcc Main.c -o Main.exe
```

### Execution
Run the executable directly from your terminal/command prompt:
```bash
./Main.exe
```
