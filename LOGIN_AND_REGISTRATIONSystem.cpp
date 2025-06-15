#include <iostream>
#include <string>
#include <regex>
#include <conio.h>  // For password masking on Windows
#include <sqlite3.h>
#include <vector>
#include <iomanip>
#include <sstream>
#include <ctime>

using namespace std;

class LoginRegistrationSystem {
private:
    sqlite3* db;
    const string DB_NAME = "users.db";
    
    // Initialize database and create tables
    bool initDatabase() {
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        
        if (rc) {
            cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
            return false;
        }
        
        // Create users table if it doesn't exist
        const char* createTableSQL = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                registration_date DATETIME DEFAULT CURRENT_TIMESTAMP,
                last_login DATETIME,
                is_active INTEGER DEFAULT 1,
                failed_attempts INTEGER DEFAULT 0
            );
        )";
        
        char* errMsg = 0;
        rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);
        
        if (rc != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
            return false;
        }
        
        return true;
    }
    
    // Close database connection
    void closeDatabase() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }
    
    // Validate username format
    bool isValidUsername(const string& username) {
        // Username should be 3-20 characters, alphanumeric and underscore only
        regex usernamePattern("^[a-zA-Z0-9_]{3,20}$");
        return regex_match(username, usernamePattern);
    }
    
    // Validate password strength
    bool isValidPassword(const string& password) {
        // Password should be at least 8 characters with at least one letter, one number, and one special character
        if (password.length() < 8) {
            return false;
        }
        
        bool hasLetter = false;
        bool hasDigit = false;
        bool hasSpecial = false;
        
        for (char c : password) {
            if (isalpha(c)) hasLetter = true;
            else if (isdigit(c)) hasDigit = true;
            else if (ispunct(c)) hasSpecial = true;
        }
        
        return hasLetter && hasDigit && hasSpecial;
    }
    
    // Simple password hashing (XOR encryption for demonstration)
    // In production, use proper hashing like bcrypt or argon2
    string hashPassword(const string& password) {
        string hashed = password;
        const int key = 42; // Simple key for XOR
        
        for (char& c : hashed) {
            c = c ^ key;
        }
        
        // Convert to hex for storage
        stringstream ss; // stringstream is mainly used for:Combining, formatting, and converting different types into a single string
.
        for (unsigned char c : hashed) {
            ss << hex << setw(2) << setfill('0') << (int)c;
        }
        
        return ss.str();// will output the string in hexadecimal format
    }
    
    // Convert hex string back to original format
    string unhashPassword(const string& hexHash) {
        string unhashed;
        for (size_t i = 0; i < hexHash.length(); i += 2) {
            string byteString = hexHash.substr(i, 2);
            char byte = (char)strtol(byteString.c_str(), nullptr, 16);
            unhashed.push_back(byte ^ 42); // XOR with same key
        }
        return unhashed;
    }
    
    // Get password input with masking
    string getPasswordInput() {
        string password;
        char ch;
        
        while ((ch = _getch()) != '\r') { // '\r' is Enter key and _getch() is used for password masking
            if (ch == '\b') { // Backspace
                if (!password.empty()) {
                    password.pop_back();
                    cout << "\b \b";
                }
            } else if (ch >= 32 && ch <= 126) { // Printable characters
                password += ch;
                cout << '*';
            }
        }
        cout << endl;
        return password;
    }
    
    // Check if username exists in database
    bool userExists(const string& username) {
        const char* sql = "SELECT COUNT(*) FROM users WHERE username = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count > 0;
    }
    
    // Insert new user into database
    bool insertUser(const string& username, const string& password) {
        const char* sql = R"(
            INSERT INTO users (username, password_hash, registration_date) 
            VALUES (?, ?, datetime('now'));
        )";
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        
        if (rc != SQLITE_OK) {
            return false;
        }
        
        string hashedPassword = hashPassword(password);
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE;
    }
    
    // Verify user credentials
    bool verifyCredentials(const string& username, const string& password) {
        const char* sql = "SELECT password_hash, is_active FROM users WHERE username = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        bool isValid = false;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            string storedHash = (char*)sqlite3_column_text(stmt, 0);
            int isActive = sqlite3_column_int(stmt, 1);
            
            if (isActive) {
                string hashedPassword = hashPassword(password);
                isValid = (storedHash == hashedPassword);
            }
        }
        
        sqlite3_finalize(stmt);
        
        // Update last login if credentials are valid
        if (isValid) {
            updateLastLogin(username);
        } else {
            incrementFailedAttempts(username);
        }
        
        return isValid;
    }
    
    // Update last login timestamp
    void updateLastLogin(const string& username) {
        const char* sql = "UPDATE users SET last_login = datetime('now'), failed_attempts = 0 WHERE username = ?;";
        sqlite3_stmt* stmt;
        
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    // Increment failed login attempts
    void incrementFailedAttempts(const string& username) {
        const char* sql = "UPDATE users SET failed_attempts = failed_attempts + 1 WHERE username = ?;";
        sqlite3_stmt* stmt;
        
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    // Get user statistics
    void getUserStats(const string& username) {
        const char* sql = R"(
            SELECT registration_date, last_login, failed_attempts 
            FROM users WHERE username = ?;
        )";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            return;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            cout << "\n=== USER PROFILE ===" << endl;
            cout << "Username: " << username << endl;
            cout << "Registration Date: " << (char*)sqlite3_column_text(stmt, 0) << endl;
            
            const char* lastLogin = (char*)sqlite3_column_text(stmt, 1);
            if (lastLogin) {
                cout << "Last Login: " << lastLogin << endl;
            } else {
                cout << "Last Login: Never" << endl;
            }
            
            int failedAttempts = sqlite3_column_int(stmt, 2);
            cout << "Failed Login Attempts: " << failedAttempts << endl;
        }
        
        sqlite3_finalize(stmt);
    }
    
public:
    // Constructor
    LoginRegistrationSystem() : db(nullptr) {
        if (!initDatabase()) {
            cerr << "Failed to initialize database!" << endl;
        }
    }
    
    // Destructor
    ~LoginRegistrationSystem() {
        closeDatabase();
    }
    
    // Registration function
    void registerUser() {
        system("cls"); // Clear screen on Windows
        cout << "=== USER REGISTRATION ===" << endl << endl;
        
        string username, password, confirmPassword;
        
        // Get username
        cout << "Enter username (3-20 characters, alphanumeric and underscore only): ";
        getline(cin, username);
        
        // Validate username format
        if (!isValidUsername(username)) {
            cout << "\nError: Invalid username format!" << endl;
            cout << "Username must be 3-20 characters long and contain only letters, numbers, and underscores." << endl;
            cout << "\nPress any key to continue...";
            _getch();
            return;
        }
        
        // Check for duplicate username
        if (userExists(username)) {
            cout << "\nError: Username '" << username << "' already exists!" << endl;
            cout << "Please choose a different username." << endl;
            cout << "\nPress any key to continue...";
            _getch();
            return;
        }
        
        // Get password
        cout << "Enter password (minimum 8 characters with letter, number, and special character): ";
        password = getPasswordInput();
        
        // Validate password
        if (!isValidPassword(password)) {
            cout << "\nError: Password does not meet requirements!" << endl;
            cout << "Password must be at least 8 characters long with at least one letter, one number, and one special character." << endl;
            cout << "\nPress any key to continue...";
            _getch();
            return;
        }
        
        // Confirm password
        cout << "Confirm password: ";
        confirmPassword = getPasswordInput();
        
        if (password != confirmPassword) {
            cout << "\nError: Passwords do not match!" << endl;
            cout << "\nPress any key to continue...";
            _getch();
            return;
        }
        
        // Save user to database
        if (insertUser(username, password)) {
            cout << "\nSuccess: User '" << username << "' registered successfully!" << endl;
            cout << "You can now login with your credentials." << endl;
        } else {
            cout << "\nError: Failed to register user. Please try again." << endl;
        }
        
        cout << "\nPress any key to continue...";
        _getch();
    }
    
    // Login function
    void loginUser() {
        system("cls"); // Clear screen on Windows
        cout << "=== USER LOGIN ===" << endl << endl;
        
        string username, password;
        
        // Get username
        cout << "Enter username: ";
        getline(cin, username);
        
        // Check if user exists
        if (!userExists(username)) {
            cout << "\nError: Username '" << username << "' not found!" << endl;
            cout << "Please register first or check your username." << endl;
            cout << "\nPress any key to continue...";
            _getch();
            return;
        }
        
        // Get password
        cout << "Enter password: ";
        password = getPasswordInput();
        
        // Verify credentials
        if (verifyCredentials(username, password)) {
            cout << "\nSuccess: Login successful!" << endl;
            cout << "Welcome back, " << username << "!" << endl;
            
            // Show user profile
            getUserStats(username);
            
            cout << "\n=== USER DASHBOARD ===" << endl;
            cout << "You are now logged in to the system." << endl;
            cout << "Available features:" << endl;
            cout << "- Secure user authentication" << endl;
            cout << "- User profile management" << endl;
            cout << "- Login history tracking" << endl;
            cout << "- Account security monitoring" << endl;
        } else {
            cout << "\nError: Invalid password!" << endl;
            cout << "Please check your password and try again." << endl;
        }
        
        cout << "\nPress any key to continue...";
        _getch();
    }
    
    // Display all registered users (admin function)
    void displayAllUsers() {
        system("cls");
        cout << "=== REGISTERED USERS ===" << endl << endl;
        
        const char* sql = R"(
            SELECT username, registration_date, last_login, failed_attempts, is_active 
            FROM users 
            ORDER BY registration_date DESC;
        )";
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        
        if (rc != SQLITE_OK) {
            cout << "Error retrieving users from database." << endl;
            cout << "\nPress any key to continue...";
            _getch();
            return;
        }
        
        int userCount = 0;
        cout << left << setw(20) << "Username" 
             << setw(20) << "Registration" 
             << setw(20) << "Last Login" 
             << setw(10) << "Failed" 
             << setw(8) << "Active" << endl;
        cout << string(78, '-') << endl;
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            userCount++;
            string username = (char*)sqlite3_column_text(stmt, 0);
            string regDate = (char*)sqlite3_column_text(stmt, 1);
            const char* lastLogin = (char*)sqlite3_column_text(stmt, 2);
            int failedAttempts = sqlite3_column_int(stmt, 3);
            int isActive = sqlite3_column_int(stmt, 4);
            
            cout << left << setw(20) << username
                 << setw(20) << regDate.substr(0, 16) // Show only date and time, not seconds
                 << setw(20) << (lastLogin ? string(lastLogin).substr(0, 16) : "Never")
                 << setw(10) << failedAttempts
                 << setw(8) << (isActive ? "Yes" : "No") << endl;
        }
        
        sqlite3_finalize(stmt);
        
        if (userCount == 0) {
            cout << "No users registered yet." << endl;
        } else {
            cout << "\nTotal registered users: " << userCount << endl;
        }
        
        cout << "\nPress any key to continue...";
        _getch();
    }
    
    // Database management function
    void manageDatabaseStats() {
        system("cls");
        cout << "=== DATABASE STATISTICS ===" << endl << endl;
        
        // Get total users
        const char* totalUsersSQL = "SELECT COUNT(*) FROM users;";
        sqlite3_stmt* stmt;
        
        sqlite3_prepare_v2(db, totalUsersSQL, -1, &stmt, NULL);
        sqlite3_step(stmt);
        int totalUsers = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        
        // Get active users
        const char* activeUsersSQL = "SELECT COUNT(*) FROM users WHERE is_active = 1;";
        sqlite3_prepare_v2(db, activeUsersSQL, -1, &stmt, NULL);
        sqlite3_step(stmt);
        int activeUsers = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        
        // Get users with recent login (last 7 days)
        const char* recentLoginsSQL = "SELECT COUNT(*) FROM users WHERE last_login >= datetime('now', '-7 days');";
        sqlite3_prepare_v2(db, recentLoginsSQL, -1, &stmt, NULL);
        sqlite3_step(stmt);
        int recentLogins = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        
        cout << "Total Users: " << totalUsers << endl;
        cout << "Active Users: " << activeUsers << endl;
        cout << "Recent Logins (7 days): " << recentLogins << endl;
        cout << "Database File: " << DB_NAME << endl;
        
        cout << "\nPress any key to continue...";
        _getch();
    }
    
    // Main menu
    void showMenu() {
        int choice;
        
        while (true) {
            system("cls");
            cout << "=====================================" << endl;
            cout << "    LOGIN & REGISTRATION SYSTEM     " << endl;
            cout << "         (SQLite Database)          " << endl;
            cout << "=====================================" << endl << endl;
            cout << "1. Register New User" << endl;
            cout << "2. Login" << endl;
            cout << "3. View All Users (Admin)" << endl;
            cout << "4. Database Statistics" << endl;
            cout << "5. Exit" << endl << endl;
            cout << "Enter your choice (1-5): ";
            
            cin >> choice;
            cin.ignore(); // Clear the input buffer
            
            switch (choice) {
                case 1:
                    registerUser();
                    break;
                case 2:
                    loginUser();
                    break;
                case 3:
                    displayAllUsers();
                    break;
                case 4:
                    manageDatabaseStats();
                    break;
                case 5:
                    system("cls");
                    cout << "Thank you for using the Login & Registration System!" << endl;
                    cout << "Database connection closed successfully." << endl;
                    cout << "Goodbye!" << endl;
                    return;
                default:
                    cout << "\nInvalid choice! Please select a number between 1-5." << endl;
                    cout << "Press any key to continue...";
                    _getch();
                    break;
            }
        }
    }
};

int main() {
    LoginRegistrationSystem system;
    system.showMenu();
    return 0;
}