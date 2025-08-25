/*
 * Project: Contact Management System
 *
 * Description:
 * This is a console-based Contact Management System implemented in C, designed to manage a
 * collection of contacts, each with a name, phone number, and email address. The system provides a
 * menu-driven interface with the following features:
 *
 * - Core Features:
 *   • Add Contact:
 *     - Prompts user for Name, Phone, and Email.
 *     - Each field is validated with regex:
 *       · Name → letters/spaces only, max length enforced.
 *       · Phone → Indian and International number format.
 *       · Email → must match standard email pattern.
 *     - Input sanitization:
 *       · Trims whitespace.
 *       · Replaces commas (to maintain CSV compatibility).
 *     - Adds contact if valid and space is available.
 *
 *   • View Contacts:
 *     - Displays all contacts in a formatted table:
 *       · Columns: Index | Name | Phone | Email
 *       - Empty fields shown as blank.
 *
 *   • Search Contacts:
 *     - Allows search by full or partial name (case-insensitive).
 *     - Matches displayed in table format.
 *
 *   • Update Contact:
 *     - Prompts for existing contact by name.
 *     - Allows updating Name / Phone / Email individually.
 *     - Empty input means "keep old value".
 *     - Validation is applied on new values.
 *
 *   • Delete Contact:
 *     - Prompts for contact name.
 *     - If found, asks for confirmation (Y/N).
 *     - On confirmation, removes contact and shifts array.
 *
 *   • Sort Contacts:
 *     - Sorts contacts in ascending order by chosen field:
 *       · Name
 *       · Phone
 *       · Email
 *     - Uses Merge Sort (efficient O(n log n)).
 *
 * - File Persistence:
 *   • Save Contacts:
 *     - Saves all contacts to "contacts.txt" (CSV format).
 *     - Each line: name,phone,email
 *
 *   • Load Contacts:
 *     - Loads existing contacts from "contacts.txt" at startup.
 *     - Ensures max contact limit not exceeded.
 *
 * - vCard (VCF) Integration:
 *   • Export to vCard:
 *     - Writes contacts to "contacts.vcf" in vCard 3.0 format.
 *     - Example:
 *       BEGIN:VCARD
 *       VERSION:3.0
 *       FN:<Name>
 *       TEL;TYPE=CELL:<Phone>
 *       EMAIL;TYPE=WORK:<Email>
 *       END:VCARD
 *     - Only one Phone/Email per contact is exported.
 *     - Phone = CELL, Email = WORK (fixed types).
 *
 *   • Import from vCard:
 *     - Reads one or more VCARDs from file.
 *     - Extracts FN, TEL, EMAIL lines.
 *     - Only last TEL and EMAIL are stored (if multiple).
 *     - On END:VCARD → adds new contact to memory.
 *     - No duplicate prevention or extra validation.
 *     - Stops if MAX_CONTACTS is reached.
 *
 * - Input Validation:
 *   • Regex is used to validate formats:
 *     - NAME_REGEX
 *     - PHONE_REGEX
 *     - EMAIL_REGEX
 *   • Invalid inputs trigger re-prompt until corrected.
 *
 * - Sanitization Helpers:
 *   • trim_whitespace → removes leading/trailing spaces.
 *   • replace_commas  → replaces commas (CSV safe).
 *   • sanitize_contact → applies both to all fields.
 *
 * - User Interface:
 *   • Menu-driven system:
 *       1. Add Contact
 *       2. View Contacts
 *       3. Search Contacts
 *       4. Delete Contact
 *       5. Update Contact
 *       6. Sort Contacts
 *       7. Export Contacts (VCF)
 *       8. Import Contacts (VCF)
 *       9. Exit
 *   • Emoji-based feedback for user actions (✅, ❌, ℹ️).
 *
 * - Technical Notes:
 *   • Stores contacts in memory (array of structs).
 *   • Max capacity: 100 contacts.
 *   • Field length limits prevent buffer overflow.
 *   • Case-insensitive name comparison via strcasecmp().
 *   • Error handling ensures stability during file I/O.
 */

#include <ctype.h>   // Provides isspace() for whitespace trimming
#include <regex.h>   // Provides regex functions for input validation
#include <stdio.h>   // Standard I/O functions like printf, scanf, fopen
#include <stdlib.h>  // Provides memory management (malloc, free) and exit
#include <string.h>  // String manipulation functions (strlen, strcpy, etc.)
#include <strings.h> // Provides strcasecmp for case-insensitive string comparison

#ifdef _WIN32
#define strcasecmp _stricmp // On Windows, strcasecmp() is not available; use _stricmp instead
#endif                      // On Linux/Unix/macOS, strcasecmp() exists, so no change

#define MAX_CONTACTS 100     // Maximum number of contacts allowed
#define MAX_NAME_LENGTH 50   // Maximum length for contact name
#define MAX_PHONE_LENGTH 17  // Maximum length for phone number (16 + '\0')
#define MAX_EMAIL_LENGTH 254 // Maximum length for email address (RFC-ish max)

#define NAME_REGEX                                                                                 \
    "^[A-Za-z][A-Za-z '-]{0,48}[A-Za-z]$" // Regex for valid name (letters, spaces, hyphens,
                                          // apostrophes)
#define EMAIL_REGEX "^[a-z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$" // Regex for valid email format
#define PHONE_REGEX                                                                                \
    "^((\\+91[6-9][0-9]{9})|([6-9][0-9]{9})|(\\+[1-9][0-9]{6,14}))$" // Regex for valid phone
                                                                     // numbers
#define CONFIRM_REGEX "^[yYnN]$"    // Regex for y/n confirmation input
#define SORT_CHOICE_REGEX "^[1-3]$" // Regex for sort choice (1-3)

// Function prototypes for contact management operations
void add_contacts(void);    // Adds a new contact
void view_contacts(void);   // Displays all contacts
void search_contact(void);  // Searches for contacts by name
void show_menu(void);       // Displays the main menu
void delete_contacts(void); // Deletes a contact by name
void update_contact(void);  // Updates an existing contact
void load_contacts(void);   // Loads contacts from file
void save_contacts(void);   // Saves contacts to file
int get_menu_choice(void);  // Gets and validates menu choice
void sort_contacts(void);   // Sorts contacts based on user choice

typedef struct
{
    char name[MAX_NAME_LENGTH];   // Contact name
    char phone[MAX_PHONE_LENGTH]; // Contact phone number
    char email[MAX_EMAIL_LENGTH]; // Contact email address
} Contact;

Contact contacts[MAX_CONTACTS]; // Global array to store contacts
int contact_count = 0;          // Tracks number of contacts

static void
trim_whitespace(char *s); // Removes leading and trailing whitespace characters from a string
static void
replace_commas(char *s); // Replaces commas in a string (to avoid format issues in CSV/VCF)
static void sanitize_contact(Contact *c); // Cleans up a contact's fields (name, phone, email) by
                                          // applying trimming/replacement
void export_to_vcf(const char *filename); // Exports all saved contacts to a VCF (vCard) file
void import_from_vcf(
    const char *filename); // Imports contacts from a VCF (vCard) file into the contact list

// Helper function prototypes for input handling and sorting
void get_input(const char *prompt, char *buffer, size_t size); // Reads input safely
void get_validated_input(const char *prompt, char *buffer, size_t size, const char *pattern,
                         int allow_empty); // Validates input with regex
void get_valid_input(const char *prompt, char *buffer, size_t size,
                     const char *pattern); // Strict input validation (no empty input)
void get_optional_valid_input(const char *prompt, char *buffer, size_t size,
                              const char *pattern); // Optional input validation (allows empty)

typedef enum {
    SORT_BY_NAME,  // Sort by contact name
    SORT_BY_PHONE, // Sort by phone number
    SORT_BY_EMAIL  // Sort by email address
} SortField;

void merge(Contact arr[], int left, int mid, int right,
           SortField field); // Merges two sorted subarrays
void merge_sort(Contact arr[], int left, int right,
                SortField field); // Implements merge sort for contacts

// Main function: program entry point
int main(void)
{
    load_contacts(); // Load contacts from file at startup
    int choice;

    printf("📱 Contact Management System Started 📱\n"); // Welcome message

    do
    {
        show_menu();                // Display the menu
        choice = get_menu_choice(); // Get user's menu choice

        switch (choice) // Handle menu choice
        {
            case 1:
                add_contacts(); // Add a new contact
                break;
            case 2:
                view_contacts(); // Display all contacts
                break;
            case 3:
                search_contact(); // Search for a contact
                break;
            case 4:
                delete_contacts(); // Delete a contact
                break;
            case 5:
                update_contact(); // Update a contact
                break;
            case 6:
                sort_contacts(); // Sort contacts
                break;
            case 7:
                export_to_vcf("contacts.vcf"); // Export Contacts
                break;
            case 8:
                import_from_vcf("Contacts1.vcf"); // Import contacts
                break;
            case 9:
                printf("Exiting the program. Goodbye!\n"); // Exit message
                break;
            default:
                printf("Invalid choice. Please try again.\n"); // Handle invalid choice
        }
    }
    while (choice != 9); // Continue until user chooses to exit
    save_contacts(); // Save contacts to file before exiting
}

void load_contacts(void) {
    FILE *file = fopen("contacts.txt", "r");
    if (!file) { printf("📂 No contacts file found. Starting fresh.\n"); return; }

    contact_count = 0;
    char line[512]; // was 100
    while (fgets(line, sizeof(line), file) && contact_count < MAX_CONTACTS) {
        line[strcspn(line, "\n")] = '\0';
        char fmt[96];
        snprintf(fmt, sizeof(fmt), "%%%d[^,], %%%d[^,], %%%d[^\n]",
                 MAX_NAME_LENGTH - 1, MAX_PHONE_LENGTH - 1, MAX_EMAIL_LENGTH - 1);

        if (sscanf(line, fmt,
                   contacts[contact_count].name,
                   contacts[contact_count].phone,
                   contacts[contact_count].email) != 3) {
            printf("Warning: Skipping malformed line: '%s'\n", line);
            continue;
        }

        sanitize_contact(&contacts[contact_count]);
        if (!validate_with_regex(NAME_REGEX, contacts[contact_count].name) ||
            !validate_with_regex(PHONE_REGEX, contacts[contact_count].phone) ||
            !validate_with_regex(EMAIL_REGEX, contacts[contact_count].email)) {
            printf("Warning: Invalid data, skipping: '%s'\n", line);
            continue;
        }

        contact_count++;
    }
    fclose(file);
    printf("📁 %d contact(s) loaded from file.\n", contact_count);
}

void import_from_vcf(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) { printf("❌ Could not open %s for reading.\n", filename); return; }

    int before = contact_count;
    char line[512], name[MAX_NAME_LENGTH]="", phone[MAX_PHONE_LENGTH]="", email[MAX_EMAIL_LENGTH]="";

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;
        if (!strncmp(line, "FN:", 3)) {
            snprintf(name, sizeof(name), "%s", line + 3);
        } else if (!strncmp(line, "TEL", 3)) {
            char *p = strchr(line, ':'); if (p) snprintf(phone, sizeof(phone), "%s", p + 1);
        } else if (!strncmp(line, "EMAIL", 5)) {
            char *p = strchr(line, ':'); if (p) snprintf(email, sizeof(email), "%s", p + 1);
        } else if (!strncmp(line, "END:VCARD", 9)) {
            if (contact_count < MAX_CONTACTS) {
                Contact c; snprintf(c.name,sizeof(c.name),"%s",name);
                snprintf(c.phone,sizeof(c.phone),"%s",phone);
                snprintf(c.email,sizeof(c.email),"%s",email);
                sanitize_contact(&c);
                if (validate_with_regex(NAME_REGEX, c.name) &&
                    validate_with_regex(PHONE_REGEX, c.phone) &&
                    validate_with_regex(EMAIL_REGEX, c.email)) {
                    contacts[contact_count++] = c;
                }
            }
            name[0]=phone[0]=email[0]='\0';
        }
    }
    fclose(file);
    printf("✅ Imported %d contacts from %s\n", contact_count - before, filename);
}


// ----------------- Sanitization helpers -----------------

// Trims leading and trailing whitespace from a string in-place
static void trim_whitespace(char *s)
{
    if (!s)
        return; // Check for NULL pointer

    // Skip leading whitespace
    char *start = s;
    while (*start && isspace((unsigned char) *start))
        start++;

    // Shift string left to remove leading whitespace
    if (start != s)
        memmove(s, start, strlen(start) + 1);

    // Trim trailing whitespace
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char) s[len - 1]))
    {
        s[--len] = '\0';
    }
}

// Replaces commas with spaces to ensure CSV compatibility
static void replace_commas(char *s)
{
    if (!s)
        return; // Check for NULL pointer
    for (; *s; ++s)
    {
        if (*s == ',')
            *s = ' '; // Replace comma with space
    }
}

// Sanitizes a contact by trimming whitespace and removing commas
static void sanitize_contact(Contact *c)
{
    if (!c)
        return;                // Check for NULL pointer
    trim_whitespace(c->name);  // Trim name
    trim_whitespace(c->phone); // Trim phone
    trim_whitespace(c->email); // Trim email
    replace_commas(c->name);   // Remove commas from name
    replace_commas(c->phone);  // Remove commas from phone
    replace_commas(c->email);  // Remove commas from email
}

// ----------------- Regex util -----------------

// Validates input against a regex pattern
int validate_with_regex(const char *pattern, const char *input)
{
    regex_t regex; // Regex object
    int result;

    // Compile the regex pattern
    result = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (result)
    {
        printf("Could not compile regex.\n"); // Handle compilation failure
        return 0;
    }

    // Execute regex against input
    result = regexec(&regex, input, 0, NULL, 0);
    regfree(&regex);    // Free regex resources
    return result == 0; // Return 1 if match, 0 if no match
}

// ----------------- File save/load -----------------

// Saves contacts to a file
void save_contacts()
{
    const char *path = "contacts.txt"; // Output file path
    const char *tmpp = "contacts.tmp"; // Temporary file path
    FILE *file = fopen(tmpp, "w");     // Open temp file for writing
    if (!file)
    {
        printf("❌ Error opening temp file.\n"); // Handle file open failure
        return;
    }

    for (int i = 0; i < contact_count; i++)
    {
        // Sanitize contact before saving
        sanitize_contact(&contacts[i]);
        // Write contact to file in CSV format
        fprintf(file, "%s, %s, %s\n", contacts[i].name, contacts[i].phone, contacts[i].email);
    }
    fclose(file); // Close the temp file

    // Replace original file with temp file
    if (remove(path) != 0)
    { /* ignore if not exists */
    }
    if (rename(tmpp, path) != 0)
    {
        printf("❌ Error finalizing save.\n"); // Handle rename failure
    }
    else
    {
        printf("✅ Contacts saved successfully to file!\n"); // Success message
    }
}

//---------------------- Load contacts------------------------
// Loads contacts from a file
void load_contacts(void)
{
    FILE *file = fopen("contacts.txt", "r"); // Open file for reading
    if (file == NULL)
    {
        printf("📂 No contacts file found. Starting fresh.\n"); // Handle missing file
        return;
    }

    contact_count = 0; // Reset contact count

    char line[100]; // Buffer for reading lines
    while (fgets(line, sizeof(line), file) != NULL && contact_count < MAX_CONTACTS)
    {
        // Remove trailing newline
        line[strcspn(line, "\n")] = '\0';

        // Build the sscanf format string dynamically so it always matches the macros
        char fmt[64];
        snprintf(fmt, sizeof(fmt), "%%%d[^,], %%%d[^,], %%%d[^\n]", MAX_NAME_LENGTH - 1,
                 MAX_PHONE_LENGTH - 1, MAX_EMAIL_LENGTH - 1);

        int result = sscanf(line, fmt, contacts[contact_count].name, contacts[contact_count].phone,
                            contacts[contact_count].email);

        if (result != 3)
        {
            printf("Warning: Skipping malformed line in contacts.txt: '%s'\n",
                   line); // Handle malformed line
            continue;
        }

        // Sanitize loaded contact
        sanitize_contact(&contacts[contact_count]);

        // Validate loaded data
        if (!validate_with_regex(NAME_REGEX, contacts[contact_count].name) ||
            !validate_with_regex(PHONE_REGEX, contacts[contact_count].phone) ||
            !validate_with_regex(EMAIL_REGEX, contacts[contact_count].email))
        {
            printf("Warning: Invalid data in line, skipping: '%s'\n", line); // Skip invalid contact
            continue;                                                        // Skip invalid contact
        }

        contact_count++;

        if (contact_count >= MAX_CONTACTS)
        {
            printf(
                "⚠️ Reached maximum contact limit while loading from file.\n"); // Handle max limit
            break;
        }
    }

    fclose(file);                                                  // Close the file
    printf("📁 %i contact(s) loaded from file.\n", contact_count); // Report loaded contacts
}

// ----------------- Menu + input validation -----------------

// Displays the main menu
void show_menu(void)
{
    printf("//--------Menu---------//\n");   // Menu header
    printf("1. Add Contact\n");              // Option 1
    printf("2. View Contacts\n");            // Option 2
    printf("3. Search Contacts\n");          // Option 3
    printf("4. Delete Contacts\n");          // Option 4
    printf("5. Update Contact\n");           // Option 5
    printf("6. Sort Contacts\n");            // Option 6
    printf("7. Export Contacts as VCF\n");   // Option 7
    printf("8. Import Contacts from VCF\n"); // Option 8
    printf("9. Exit\n");                     // Option 9
}

// Gets and validates user menu choice
int get_menu_choice(void)
{
    int choice;
    char buffer[50]; // Buffer for input

    while (1) // Loop until valid input
    {
        printf("Enter your choice: "); // Prompt for input

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            // Remove trailing newline
            buffer[strcspn(buffer, "\n")] = '\0';

            // Check for empty input
            if (strlen(buffer) == 0)
            {
                printf("Input cannot be empty. Please try again.\n"); // Handle empty input
                continue;
            }

            // Convert input to number
            char *endptr;
            choice = strtol(buffer, &endptr, 10);
            if (*endptr != '\0')
            {
                printf("Invalid input. Please enter a number.\n"); // Handle non-numeric input
                continue;
            }

            // Check range
            if (choice < 1 || choice > 9)
            {
                printf("Choice out of range. Please enter 1-9.\n"); // Handle out-of-range input
                continue;
            }

            return choice; // Return valid choice
        }
        else
        {
            printf("Error reading input. Please try again.\n"); // Handle input error
            clearerr(stdin);                                    // Clear input error state
        }
    }
}

// ----------------- Helper input functions -----------------

// Safe line input (fgets + trim newline)
void get_input(const char *prompt, char *buffer, size_t size)
{
    // Prompt
    printf("%s", prompt);

    // Use fgets to allow spaces
    if (fgets(buffer, (int) size, stdin) == NULL)
    {
        // if EOF or error, ensure buffer is empty string
        buffer[0] = '\0';
        return;
    }

    // If input didn’t fit into buffer completely, flush leftover
    if (strchr(buffer, '\n') == NULL)
    {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ; // flush until newline
    }

    // Trim trailing newline if present
    buffer[strcspn(buffer, "\n")] = '\0';

    trim_whitespace(buffer);
}

// Core function for validated input
// allow_empty = 0 → empty input is rejected
// allow_empty = 1 → empty input is accepted
// Validate input with regex, optionally allow empty input
void get_validated_input(const char *prompt, char *buffer, size_t size, const char *pattern,
                         int allow_empty)
{
    while (1)
    {
        get_input(prompt, buffer, size);

        // Empty input handling
        if (buffer[0] == '\0')
        {
            if (allow_empty)
                return; // accept empty if allowed
            printf("❌ Input cannot be empty. Please try again.\n");
            continue;
        }

        // Length check
        if (strlen(buffer) >= size)
        {
            printf("❌ Input too long. Maximum length is %zu characters.\n", size - 1);
            continue;
        }

        // Validate regex if provided
        if (pattern == NULL || validate_with_regex(pattern, buffer))
        {
            return; // ✅ Valid input
        }

        // Give user an idea of expected format
        const char *format_msg;
        if (strcmp(pattern, NAME_REGEX) == 0)
        {
            format_msg = "Letters, spaces, hyphens, or apostrophes (1-49 chars)";
        }
        else if (strcmp(pattern, PHONE_REGEX) == 0)
        {
            format_msg = "10-15 digits, optional + e.g., (International: +14155552671), (Indian: "
                         "+919876543210 or 9876543210)";
        }
        else if (strcmp(pattern, CONFIRM_REGEX) == 0)
        {
            format_msg = "Single character: 'y' or 'n'";
        }
        else if (strcmp(pattern, "^[1-2]$") == 0)
        {
            format_msg = "1 or 2";
        }
        else if (strcmp(pattern, SORT_CHOICE_REGEX) == 0)
        {
            if (!validate_with_regex("^[0-9]+$", buffer))
            {
                format_msg = "❌ Invalid input. Enter a number (1-3).";
            }
            else
            {
                format_msg = "❌ Choice out of range. Enter between 1 and 3.";
            }
        }
        else
        {
            format_msg = "Valid email (e.g., user@domain.com)";
        }
        printf("Expected format: %s\n", format_msg);
    }
}

// Strict input validation (no empty input allowed)
void get_valid_input(const char *prompt, char *buffer, size_t size, const char *pattern)
{
    get_validated_input(prompt, buffer, size, pattern,
                        0); // Call validated input with no empty allowed
}

// Optional input validation (empty input allowed)
void get_optional_valid_input(const char *prompt, char *buffer, size_t size, const char *pattern)
{
    get_validated_input(prompt, buffer, size, pattern,
                        1); // Call validated input with empty allowed
}

// ----------------- Add contact -----------------

// Adds a new contact to the contacts array
void add_contacts(void)
{
    if (contact_count >= MAX_CONTACTS)
    {
        printf("Contact list is full. Cannot add more contacts\n"); // Handle full contact list
        return;
    }

    // Get and validate contact details
    get_valid_input("Enter name (1-49 chars): ", contacts[contact_count].name, MAX_NAME_LENGTH,
                    NAME_REGEX); // Get name
    get_valid_input(
        "Enter phone e.g., (International: +14155552671), (Indian: +919876543210 or 9876543210): ",
        contacts[contact_count].phone, MAX_PHONE_LENGTH, PHONE_REGEX); // Get phone
    get_valid_input("Enter email (e.g., user@domain.com): ", contacts[contact_count].email,
                    MAX_EMAIL_LENGTH, EMAIL_REGEX); // Get email

    contact_count++; // Increment contact count

    // Display added contact
    printf("\nContact added:\n");
    printf("%s\n", contacts[contact_count - 1].name);  // Show name
    printf("%s\n", contacts[contact_count - 1].phone); // Show phone
    printf("%s\n", contacts[contact_count - 1].email); // Show email
}

// ----------------- View contacts -----------------

// Displays all contacts in a formatted table
void view_contacts(void)
{
    if (contact_count == 0)
    {
        printf("No contacts in Contact manager, add yours :)\n"); // Handle empty contact list
        return;
    }

    // Print table header
    printf("\n📒 Contact List (%d):\n", contact_count);
    printf("-------------------------------------------------------------------------\n");
    printf("%-3s %-30s %-16s %-25s\n", "#", "Name", "Phone", "Email");
    printf("-------------------------------------------------------------------------\n");

    // Print each contact
    for (int i = 0; i < contact_count; i++)
    {
        printf("%-3d %-30.30s %-16.16s %-25.25s\n", i + 1, contacts[i].name, contacts[i].phone,
               contacts[i].email);
    }
    printf("-------------------------------------------------------------------------\n"); // Print
                                                                                           // table
                                                                                           // footer
}

// ----------------- Update contact -----------------

// Updates an existing contact's details
void update_contact(void)
{
    if (contact_count == 0)
    {
        printf("No contacts in Contact manager, add yours :)\n"); // Handle empty contact list
        return;
    }

    char name[MAX_NAME_LENGTH];
    get_valid_input("Enter name to update: ", name, MAX_NAME_LENGTH,
                    NAME_REGEX); // Get name to update

    int found = 0;
    for (int i = 0; i < contact_count; i++)
    {
        if (strcasecmp(contacts[i].name, name) == 0) // Case-insensitive name match
        {
            found = 1;                       // Mark contact as found
            printf("\n📞 Contact Found:\n"); // Display found contact
            printf("Name: %s\n", contacts[i].name);
            printf("Phone: %s\n", contacts[i].phone);
            printf("Email: %s\n\n", contacts[i].email);

            printf("Enter new details (press Enter to keep existing value)\n"); // Prompt for new
                                                                                // details

            char new_name[MAX_NAME_LENGTH];
            char new_email[MAX_EMAIL_LENGTH];
            char new_phone[MAX_PHONE_LENGTH];
            int updated = 0; // Track if updates were made

            // Update name
            get_optional_valid_input("Enter new name (1-49 chars): ", new_name, MAX_NAME_LENGTH,
                                     NAME_REGEX);
            if (new_name[0] != '\0' && strcmp(new_name, contacts[i].name) != 0)
            {
                // Check for duplicate name
                for (int j = 0; j < contact_count; j++)
                {
                    if (j != i && strcasecmp(contacts[j].name, new_name) == 0)
                    {
                        printf("Cannot update: Name '%s' already exists.\n",
                               new_name); // Handle duplicate name
                        return;
                    }
                }
                printf("Name: '%s' → '%s'\n", contacts[i].name, new_name); // Show name change
                snprintf(contacts[i].name, sizeof(contacts[i].name), "%s", new_name); // Update name
                updated = 1;
            }

            // Update phone
            get_optional_valid_input("Enter new phone e.g., (International: +14155552671), "
                                     "(Indian: +919876543210 or 9876543210): ",
                                     new_phone, MAX_PHONE_LENGTH, PHONE_REGEX);
            if (new_phone[0] != '\0' && strcmp(new_phone, contacts[i].phone) != 0)
            {
                printf("Phone: '%s' → '%s'\n", contacts[i].phone, new_phone); // Show phone change
                snprintf(contacts[i].phone, sizeof(contacts[i].phone), "%s",
                         new_phone); // Update phone
                updated = 1;
            }

            // Update email
            get_optional_valid_input("Enter new email (e.g., user@domain.com): ", new_email,
                                     MAX_EMAIL_LENGTH, EMAIL_REGEX);
            if (new_email[0] != '\0' && strcmp(new_email, contacts[i].email) != 0)
            {
                printf("Email: '%s' → '%s'\n", contacts[i].email, new_email); // Show email change
                snprintf(contacts[i].email, sizeof(contacts[i].email), "%s",
                         new_email); // Update email
                updated = 1;
            }

            if (updated)
                printf("\n✅ Contact updated successfully!\n\n"); // Confirm update
            else
            {
                printf("\nℹ️ No changes were made to the contact.\n"); // No changes made
                printf("Name: %s\n", contacts[i].name);
                printf("Phone: %s\n", contacts[i].phone);
                printf("Email: %s\n\n", contacts[i].email);
            }
            break; // Exit loop after updating
        }
    }

    if (!found)
    {
        printf("Contact '%s' not found.\n", name); // Handle contact not found
    }
}

// ----------------- Delete contacts -----------------

// Deletes a contact by name
void delete_contacts(void)
{
    if (contact_count == 0)
    {
        printf("No contacts to delete.\n"); // Handle empty contact list
        return;
    }

    char name[MAX_NAME_LENGTH];
    get_valid_input("Enter name to delete: ", name, MAX_NAME_LENGTH,
                    NAME_REGEX); // Get name to delete

    int found = 0;

    for (int i = 0; i < contact_count; i++)
    {
        if (strcasecmp(contacts[i].name, name) == 0) // Case-insensitive name match
        {
            found = 1; // Mark contact as found

            printf("\n📞 Contact Found:\n"); // Display found contact
            printf("Name: %s\n", contacts[i].name);
            printf("Phone: %s\n", contacts[i].phone);
            printf("Email: %s\n", contacts[i].email);

            char confirm[2];
            get_valid_input("Are you sure you want to delete this contact? [y/n]: ", confirm,
                            sizeof(confirm), CONFIRM_REGEX); // Get confirmation

            if (confirm[0] == 'y' || confirm[0] == 'Y')
            {
                // Shift contacts left to remove the contact
                for (int j = i; j < contact_count - 1; j++)
                {
                    contacts[j] = contacts[j + 1];
                }

                // Clear the last contact slot
                memset(&contacts[contact_count - 1], 0, sizeof(Contact));

                contact_count--;                                           // Decrease contact count
                printf("✅ Contact '%s' deleted successfully.\n\n", name); // Confirm deletion
            }
            else
            {
                printf("❌ Deletion cancelled.\n\n"); // Cancellation message
            }

            break; // Exit loop after handling contact
        }
    }

    if (!found)
    {
        printf("Contact '%s' not found.\n", name); // Handle contact not found
    }
}

// ----------------- Search contacts -----------------

// Searches for contacts by name (exact or partial match)
void search_contact(void)
{
    if (contact_count == 0)
    {
        printf("No contacts in Contact manager, add yours :)\n"); // Handle empty contact list
        return;
    }

    // Prompt for search type
    char choice_str[2];
    get_valid_input("Search type (1 = Exact, 2 = Partial): ", choice_str, sizeof(choice_str),
                    "^[1-2]$");            // Get search type
    int search_type = choice_str[0] - '0'; // Convert char to int (1 or 2)

    // Get search name
    char name[MAX_NAME_LENGTH];
    get_valid_input("Enter name to search: ", name, MAX_NAME_LENGTH,
                    NAME_REGEX); // Get name to search

    // Convert search name to lowercase for case-insensitive comparison
    char temp_search[MAX_NAME_LENGTH];
    strcpy(temp_search, name);
    for (int j = 0; temp_search[j]; j++)
    {
        temp_search[j] = tolower((unsigned char) temp_search[j]);
    }

    int found = 0;
    // Print search results header
    printf("\n📞 Search Results:\n");
    printf("---------------------------------------------------------------\n");
    printf("%-3s %-15s %-15s %-25s\n", "#", "Name", "Phone", "Email");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < contact_count; i++)
    {
        // Convert contact name to lowercase for comparison
        char temp_name[MAX_NAME_LENGTH];
        strcpy(temp_name, contacts[i].name);
        for (int j = 0; temp_name[j]; j++)
        {
            temp_name[j] = tolower((unsigned char) temp_name[j]);
        }

        int match = 0;
        if (search_type == 1)
        { // Exact match
            match = (strcasecmp(temp_name, temp_search) == 0);
        }
        else
        { // Partial match
            match = (strstr(temp_name, temp_search) != NULL);
        }

        if (match)
        {
            // Print matching contact
            printf("%-3d %-15s %-15s %-25s\n", found + 1, contacts[i].name, contacts[i].phone,
                   contacts[i].email);
            found++;
        }
    }

    printf(
        "---------------------------------------------------------------\n"); // Print table footer
    if (found == 0)
    {
        printf("Contact not found :(\n"); // Handle no matches
    }
    else
    {
        printf("Found %d contact(s).\n", found); // Report number of matches
    }
}

// ----------------- Sort contacts -----------------

// Merges two sorted subarrays based on the specified field
void merge(Contact arr[], int left, int mid, int right, SortField field)
{
    int n1 = mid - left + 1; // Size of left subarray
    int n2 = right - mid;    // Size of right subarray

    // Allocate temporary arrays
    Contact *L = malloc(n1 * sizeof(Contact));
    Contact *R = malloc(n2 * sizeof(Contact));

    // Check for allocation failure
    if (!L || !R)
    {
        free(L);                                                 // Free if allocated
        free(R);                                                 // Free if allocated
        fprintf(stderr, "Memory allocation failed in merge.\n"); // Report error
        exit(EXIT_FAILURE);
    }

    // Copy data to temporary arrays
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left; // Indices for merging

    // Merge arrays based on field
    while (i < n1 && j < n2)
    {
        int cmp;
        if (field == SORT_BY_NAME)
            cmp = strcasecmp(L[i].name, R[j].name); // Compare names
        else if (field == SORT_BY_PHONE)
            cmp = strcmp(L[i].phone, R[j].phone); // Compare phone numbers
        else
            cmp = strcasecmp(L[i].email, R[j].email); // Compare emails

        if (cmp <= 0)
            arr[k++] = L[i++]; // Copy from left subarray
        else
            arr[k++] = R[j++]; // Copy from right subarray
    }

    // Copy remaining elements from left subarray
    while (i < n1)
        arr[k++] = L[i++];
    // Copy remaining elements from right subarray
    while (j < n2)
        arr[k++] = R[j++];

    // Free temporary arrays
    free(L);
    free(R);
}

// Implements merge sort on contacts array
void merge_sort(Contact arr[], int left, int right, SortField field)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2; // Calculate middle index

        merge_sort(arr, left, mid, field);      // Sort left half
        merge_sort(arr, mid + 1, right, field); // Sort right half
        merge(arr, left, mid, right, field);    // Merge sorted halves
    }
}

// Sorts contacts based on user-selected field
void sort_contacts(void)
{
    if (contact_count == 0)
    {
        printf("No contacts to sort.\n"); // Handle empty contact list
        return;
    }

    // Display sort options
    printf("Sort by:\n");
    printf("1. Name\n");
    printf("2. Phone\n");
    printf("3. Email\n");

    char choice_str[3]; // Buffer for sort choice
    get_valid_input("Enter choice (1-3): ", choice_str, sizeof(choice_str),
                    SORT_CHOICE_REGEX); // Get sort choice

    int choice = choice_str[0] - '0'; // Convert char to int

    SortField field;
    switch (choice)
    {
        case 1:
            field = SORT_BY_NAME;
            break; // Set sort by name
        case 2:
            field = SORT_BY_PHONE;
            break; // Set sort by phone
        case 3:
            field = SORT_BY_EMAIL;
            break; // Set sort by email
        default:
            printf("Invalid choice.\n"); // Handle invalid choice (shouldn't occur due to regex)
            return;
    }

    merge_sort(contacts, 0, contact_count - 1, field); // Sort contacts
    printf("Contacts sorted successfully!\n");         // Confirm sort
}
