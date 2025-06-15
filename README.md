# Login and Registration System with SQLite Database

A comprehensive C++ login and registration system using SQLite database for secure user management.

## Features

### Core Functionality
- **User Registration**: Secure user account creation with validation
- **User Login**: Authentication with credential verification
- **Password Security**: XOR-based password hashing (demo implementation)
- **Input Validation**: Username and password strength requirements
- **Duplicate Prevention**: Checks for existing usernames

### Database Features
- **SQLite Integration**: Persistent data storage using SQLite database
- **User Profiles**: Detailed user information tracking
- **Login History**: Last login timestamp tracking
- **Failed Attempts**: Security monitoring for failed login attempts
- **Admin Functions**: User management and statistics

### Security Features
- **Password Masking**: Hidden password input with asterisks
- **Account Status**: Active/inactive user management
- **Failed Login Tracking**: Monitor security breaches
- **Data Integrity**: SQLite database ensures data consistency

## Requirements

### Software Dependencies
- **C++ Compiler**: GCC, Clang, or Visual Studio (C++11 or later)
- **SQLite3**: SQLite3 library and headers
- **Windows**: Uses `conio.h` for password masking (Windows-specific)

### Installing SQLite on Windows

#### Option 1: Using MSYS2/MinGW
```powershell
# Install MSYS2 first, then:
pacman -S mingw-w64-x86_64-sqlite3
```

#### Option 2: Manual Installation
1. Download SQLite from https://www.sqlite.org/download.html
2. Download the precompiled binaries and source files
3. Extract and add to your project or system PATH

#### Option 3: Using vcpkg (Visual Studio)
```powershell
vcpkg install sqlite3
```

## Compilation

### Using GCC/MinGW
```powershell
g++ -std=c++17 LOGIN_AND_REGISTRATIONSystem.cpp -o LoginSystem.exe -lsqlite3
```

### Using Visual Studio
1. Add SQLite3 library to project dependencies
2. Include SQLite3 headers in project settings
3. Build normally using Visual Studio

## Usage

### Running the Application
```powershell
./LoginSystem.exe
```

### Menu Options
1. **Register New User**: Create a new user account
2. **Login**: Authenticate existing user
3. **View All Users (Admin)**: Display all registered users
4. **Database Statistics**: Show database information and statistics
5. **Exit**: Close the application

### Password Requirements
- Minimum 8 characters
- At least one letter (a-z, A-Z)
- At least one number (0-9)
- At least one special character (!@#$%^&*...)

### Username Requirements
- 3-20 characters
- Only alphanumeric characters and underscores
- Must be unique

## Database Schema

The system creates a SQLite database file `users.db` with the following table structure:

```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    registration_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_login DATETIME,
    is_active INTEGER DEFAULT 1,
    failed_attempts INTEGER DEFAULT 0
);
```

## File Structure

```
├── LOGIN_AND_REGISTRATIONSystem.cpp    # Main source code
├── users.db                            # SQLite database (created automatically)
├── README.md                           # This documentation
└── LoginSystem.exe                     # Compiled executable
```

## Security Considerations

⚠️ **Important Security Notes**:

1. **Password Hashing**: The current implementation uses simple XOR encryption for demonstration. In production, use proper cryptographic hashing libraries like:
   - bcrypt
   - Argon2
   - PBKDF2
   - scrypt

2. **Database Security**: Consider encrypting the SQLite database file for sensitive applications.

3. **Input Sanitization**: Always validate and sanitize user inputs to prevent SQL injection attacks.

4. **Environment**: This demo is designed for educational purposes. Production systems require additional security measures.

## Example Usage

### Registration Flow
```
=== USER REGISTRATION ===

Enter username (3-20 characters, alphanumeric and underscore only): john_doe
Enter password (minimum 8 characters with letter, number, and special character): ********
Confirm password: ********

Success: User 'john_doe' registered successfully!
You can now login with your credentials.
```

### Login Flow
```
=== USER LOGIN ===

Enter username: john_doe
Enter password: ********

Success: Login successful!
Welcome back, john_doe!

=== USER PROFILE ===
Username: john_doe
Registration Date: 2025-06-15 10:30:45
Last Login: Never
Failed Login Attempts: 0
```

## Advanced Features

### Admin Functions
- View all registered users with detailed information
- Monitor failed login attempts
- Track user activity and registration dates
- Database statistics and health monitoring

### Error Handling
- Comprehensive input validation
- Database connection error handling
- User-friendly error messages
- Graceful failure recovery

## Future Enhancements

- [ ] Email verification
- [ ] Password reset functionality
- [ ] User roles and permissions
- [ ] Session management
- [ ] Two-factor authentication
- [ ] Account lockout after multiple failed attempts
- [ ] Password complexity scoring
- [ ] User profile management
- [ ] Audit logging

## Troubleshooting

### Common Issues

**SQLite not found**:
- Ensure SQLite3 is installed and accessible
- Check compiler flags and library paths

**Compilation errors**:
- Verify C++11 or later compiler support
- Check SQLite3 headers are available

**Database permission errors**:
- Ensure write permissions in application directory
- Check file system permissions for database creation

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is for educational purposes. Feel free to modify and distribute according to your needs.

## Support

For questions, issues, or improvements, please create an issue in the repository or contact the development team.
