#include <ctype.h>     // for isspace(), used by trimming helper
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   // for strcasecmp

#define MAX_CONTACTS 100
#define MAX_NAME_LENGTH 50
#define MAX_PHONE_LENGTH 15
#define MAX_EMAIL_LENGTH 25

#define NAME_REGEX "^[A-Za-z][A-Za-z '-]{0,48}[A-Za-z]$"
#define EMAIL_REGEX "^[a-z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"
#define PHONE_REGEX "^((\\+91[6-9][0-9]{9})|([6-9][0-9]{9})|(\\+[1-9][0-9]{6,14}))$"
#define CONFIRM_REGEX "^[yYnN]$"


// Function prototypes
void add_contacts(void);
void view_contacts(void);
void search_contact(void);
void show_menu(void);
void delete_contacts(void);
void update_contact(void);
void load_contacts(void);
void save_contacts(void);
int get_menu_choice(void);
void sort_contacts(void);

// New helper prototypes
void get_input(const char *prompt, char *buffer, size_t size);
void get_validated_input(const char *prompt, char *buffer, size_t size,
                         const char *pattern, int allow_empty);
void get_valid_input(const char *prompt, char *buffer, size_t size, const char *pattern);
void get_optional_valid_input(const char *prompt, char *buffer, size_t size, const char *pattern);
typedef struct
{
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    char email[MAX_EMAIL_LENGTH];
} Contact;

typedef enum {
    SORT_BY_NAME,
    SORT_BY_PHONE,
    SORT_BY_EMAIL
} SortField;

void merge(Contact arr[], int left, int mid, int right, SortField field);
void merge_sort(Contact arr[], int left, int right, SortField field);

Contact contacts[MAX_CONTACTS];
int contact_count = 0;

int main(void)
{
    load_contacts();
    int choice;

    printf("📱 Contact Management System Started 📱\n");

    do
    {
        show_menu();
        choice = get_menu_choice();

        switch (choice)
        {
            case 1:
                add_contacts();
                break;
            case 2:
                view_contacts();
                break;
            case 3:
                search_contact();
                break;
            case 4:
                delete_contacts();
                break;
            case 5:
                update_contact();
                break;
            case 6:
                sort_contacts();
                break;
            case 7:
                printf("Exiting the program. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    while (choice != 7);
    save_contacts();
}

// ----------------- Sanitization helpers -----------------

// Trim leading & trailing whitespace in-place
static void trim_whitespace(char *s) {
    if (!s) return;

    // skip leading
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;

    // shift left if needed
    if (start != s) memmove(s, start, strlen(start) + 1);

    // trim trailing
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

// Replace commas with spaces (keeps your CSV format safe)
static void replace_commas(char *s) {
    if (!s) return;
    for (; *s; ++s) {
        if (*s == ',') *s = ' ';
    }
}

// Sanitize all fields of a contact: trim then remove commas
static void sanitize_contact(Contact *c) {
    if (!c) return;
    trim_whitespace(c->name);
    trim_whitespace(c->phone);
    trim_whitespace(c->email);

    replace_commas(c->name);
    replace_commas(c->phone);
    replace_commas(c->email);
}

// ----------------- Regex util (unchanged) -----------------
int validate_with_regex(const char *pattern, const char *input) {
    regex_t regex;
    int result;

    result = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (result) {
        printf("Could not compile regex.\n");
        return 0;
    }

    result = regexec(&regex, input, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}


// ----------------- file save/load (unchanged) -----------------
void save_contacts() {
    const char *path = "contacts.txt";
    const char *tmpp = "contacts.tmp";
    FILE *file = fopen(tmpp, "w");
    if (!file) { printf("❌ Error opening temp file.\n"); return; }

    for (int i = 0; i < contact_count; i++) {
        // ensure contact is clean before writing
        sanitize_contact(&contacts[i]);
        fprintf(file, "%s, %s, %s\n",
                contacts[i].name, contacts[i].phone, contacts[i].email);
    }
    fclose(file);
    // replace original
    if (remove(path) != 0) { /* ignore if not exists */ }
    if (rename(tmpp, path) != 0) {
        printf("❌ Error finalizing save.\n");
    } else {
        printf("✅ Contacts saved successfully to file!\n");
    }
}


void load_contacts()
{
    FILE *file = fopen("contacts.txt", "r");
    if (file == NULL)
    {
        printf("📂 No contacts file found. Starting fresh.\n");
        return;
    }
    contact_count = 0; // Reset contact count before loading

    while (fscanf(file, "%49[^,], %14[^,], %24[^\n]\n",
                  contacts[contact_count].name,
                  contacts[contact_count].phone,
                  contacts[contact_count].email) == 3)
    {
        // Sanitize the freshly read contact (handles old or hand-edited files)
        sanitize_contact(&contacts[contact_count]);

        contact_count++;

        if (contact_count >= MAX_CONTACTS)
        {
            printf("⚠️ Reached maximum contact limit while loading from file.\n");
            break;
        }
    }
    fclose(file);
    printf("📁 %i contact(s) loaded from file.\n", contact_count);
}

// ----------------- Menu + input validation (unchanged) -----------------
void show_menu(void)
{
    printf("//--------Menu---------//");
    printf("\n1. Add Contact\n");
    printf("2. View Contacts\n");
    printf("3. Search Contacts\n");
    printf("4. Delete Contacts\n");
    printf("5. Update Contact\n");
    printf("6. Sort Contacts\n");
    printf("7. Exit\n");
}

int get_menu_choice(void)
{
    int choice;
    char buffer[50];

    while (1) // keep asking until valid
    {
        printf("Enter your choice: ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            // remove trailing newline
            buffer[strcspn(buffer, "\n")] = '\0';

            // check empty
            if (strlen(buffer) == 0)
            {
                printf("Input cannot be empty. Please try again.\n");
                continue;
            }

            // check if it's a number
            char *endptr;
            choice = strtol(buffer, &endptr, 10);
            if (*endptr != '\0')
            {
                printf("Invalid input. Please enter a number.\n");
                continue;
            }

            // check range
            if (choice < 1 || choice > 7)
            {
                printf("Choice out of range. Please enter 1-7.\n");
                continue;
            }

            return choice; // ✅ valid choice
        }
        else
        {
            printf("Error reading input. Please try again.\n");
            clearerr(stdin);
        }
    }
}

// ----------------- Helper input functions (new) -----------------

// Safe line input (fgets + trim newline)
void get_input(const char *prompt, char *buffer, size_t size)
{
    // Prompt
    printf("%s", prompt);

    // Use fgets to allow spaces
    if (fgets(buffer, (int)size, stdin) == NULL)
    {
        // if EOF or error, ensure buffer is empty string
        buffer[0] = '\0';
        return;
    }
    // Trim trailing newline if present
    buffer[strcspn(buffer, "\n")] = '\0';

    // <-- centralized trimming -->
    trim_whitespace(buffer);
}

// Core function for validated input
// allow_empty = 0 → empty input is rejected
// allow_empty = 1 → empty input is accepted
// Validate input with regex, optionally allow empty input
void get_validated_input(const char *prompt, char *buffer, size_t size,
                         const char *pattern, int allow_empty) {
    while (1) {
        get_input(prompt, buffer, size);

        // Empty input handling
        if (buffer[0] == '\0') {
            if (allow_empty) return; // accept empty if allowed
            printf("❌ Input cannot be empty. Please try again.\n");
            continue;
        }

        // Length check
        if (strlen(buffer) >= size) {
            printf("❌ Input too long. Maximum length is %zu characters.\n", size - 1);
            continue;
        }

        // Validate regex if provided
        if (pattern == NULL || validate_with_regex(pattern, buffer)) {
            return; // ✅ Valid input
        }

        // Invalid format → show specific guidance
        printf("❌ Invalid format. Please try again%s.\n",
               allow_empty ? " (or press Enter to keep existing)" : "");

        // Give user an idea of expected format
        const char *format_msg;
        if (strcmp(pattern, NAME_REGEX) == 0) {
            format_msg = "Letters, spaces, hyphens, or apostrophes (1-48 chars)";
        } else if (strcmp(pattern, PHONE_REGEX) == 0) {
            format_msg = "10-15 digits, optional + e.g., (International: +14155552671), (Indian: +919876543210 or 9876543210)";
        } else if (strcmp(pattern, CONFIRM_REGEX) == 0) {
            format_msg = "Single character: 'y' or 'n'";
        } else {
            format_msg = "Valid email (e.g., user@domain.com)";
        }
        printf("Expected format: %s\n", format_msg);
    }
}



// Strict — no empty allowed
void get_valid_input(const char *prompt, char *buffer, size_t size, const char *pattern)
{
    get_validated_input(prompt, buffer, size, pattern, 0);
}

// Optional — empty allowed
void get_optional_valid_input(const char *prompt, char *buffer, size_t size, const char *pattern)
{
    get_validated_input(prompt, buffer, size, pattern, 1);
}


// ----------------- Add contact (refactored to use centralized validation) -----------------
void add_contacts(void)
{
    if (contact_count >= MAX_CONTACTS)
    {
        printf("Contact list is full. Cannot add more contacts\n");
        return;
    }

    // Important: after validate_input() (which used scanf), the newline is consumed,
    // so stdin is ready for fgets here.

    // Get and validate inputs
    get_valid_input("Enter name (1-48 chars): ", contacts[contact_count].name, MAX_NAME_LENGTH, NAME_REGEX);
    get_valid_input("Enter phone e.g., (International: +14155552671), (Indian: +919876543210 or 9876543210): ", contacts[contact_count].phone, MAX_PHONE_LENGTH, PHONE_REGEX);
    get_valid_input("Enter email (e.g., user@domain.com): ", contacts[contact_count].email, MAX_EMAIL_LENGTH, EMAIL_REGEX);

    contact_count++;

    printf("\nContact added:\n");
    printf("%s\n", contacts[contact_count - 1].name);
    printf("%s\n", contacts[contact_count - 1].phone);
    printf("%s\n", contacts[contact_count - 1].email);
}

// ----------------- View contacts (unchanged, uses contacts[] populated by load) -----------------
void view_contacts(void)
{
    if (contact_count == 0)
    {
        printf("No contacts in Contact manager, add yours :)\n");
        return;
    }

    printf("\n📒 Contact List (%d):\n", contact_count);
    printf("---------------------------------------------------------------\n");
    printf("%-3s %-15s %-15s %-25s\n", "#", "Name", "Phone", "Email");
    printf("---------------------------------------------------------------\n");
    for (int i = 0; i < contact_count; i++) {
        printf("%-3d %-15s %-15s %-25s\n",
            i + 1, contacts[i].name, contacts[i].phone, contacts[i].email);
    }
    printf("---------------------------------------------------------------\n");

}

// ----------------- Update contact (refactored to use centralized validation) -----------------
void update_contact(void)
{
    if (contact_count == 0)
    {
        printf("No contacts in Contact manager, add yours :)\n");
        return;
    }

    char name[MAX_NAME_LENGTH];
    get_valid_input("Enter name to update: ", name, MAX_NAME_LENGTH, NAME_REGEX); // strict

    int found = 0;
    for (int i = 0; i < contact_count; i++)
    {
        if (strcasecmp(contacts[i].name, name) == 0)
        {
            found = 1;
            printf("\n📞 Contact Found:\n");
            printf("Name: %s\n", contacts[i].name);
            printf("Phone: %s\n", contacts[i].phone);
            printf("Email: %s\n\n", contacts[i].email);

            printf("Enter new details (press Enter to keep existing value)\n");

            char new_name[MAX_NAME_LENGTH];
            char new_email[MAX_EMAIL_LENGTH];
            char new_phone[MAX_PHONE_LENGTH];
            int updated = 0;

            // Name
            get_optional_valid_input("Enter new name (1-48 chars): ", new_name, MAX_NAME_LENGTH, NAME_REGEX);
            if (new_name[0] != '\0' && strcmp(new_name, contacts[i].name) != 0)
            {
                for (int j = 0; j < contact_count; j++) {
                if (j != i && strcasecmp(contacts[j].name, new_name) == 0) {
                        printf("Cannot update: Name '%s' already exists.\n", new_name);
                        return;
                    }
                }
                printf("Name: '%s' → '%s'\n", contacts[i].name, new_name);
                snprintf(contacts[i].name, sizeof(contacts[i].name), "%s", new_name);
                updated = 1;
            }

            // Phone
            get_optional_valid_input("Enter new phone e.g., (International: +14155552671), (Indian: +919876543210 or 9876543210): ", new_phone, MAX_PHONE_LENGTH, PHONE_REGEX);
            if (new_phone[0] != '\0' && strcmp(new_phone, contacts[i].phone) != 0)
            {
                printf("Phone: '%s' → '%s'\n", contacts[i].phone, new_phone);
                snprintf(contacts[i].phone, sizeof(contacts[i].phone), "%s", new_phone);
                updated = 1;
            }

            // Email
            get_optional_valid_input("Enter new email (e.g., user@domain.com): ", new_email, MAX_EMAIL_LENGTH, EMAIL_REGEX);
            if (new_email[0] != '\0' && strcmp(new_email, contacts[i].email) != 0)
            {
                printf("Email: '%s' → '%s'\n", contacts[i].email, new_email);
                snprintf(contacts[i].email, sizeof(contacts[i].email), "%s", new_email);
                updated = 1;
            }

            if (updated)
                printf("\n✅ Contact updated successfully!\n\n");
            else
            {
                printf("\nℹ️ No changes were made to the contact.\n");
                printf("Name: %s\n", contacts[i].name);
                printf("Phone: %s\n", contacts[i].phone);
                printf("Email: %s\n\n", contacts[i].email);
            }
            break;
        }
    }

    if (!found)
    {
        printf("Contact '%s' not found.\n", name);
    }
}



// ----------------- Delete contacts (updated to use get_input) -----------------
void delete_contacts(void)
{
    if (contact_count == 0)
    {
        printf("No contacts to delete.\n");
        return;
    }

    char name[MAX_NAME_LENGTH];
    get_valid_input("Enter name to delete: ", name, MAX_NAME_LENGTH, NAME_REGEX);

    int found = 0;

    for (int i = 0; i < contact_count; i++)
    {
        if (strcasecmp(contacts[i].name, name) == 0)
        {
            printf("\n📞 Contact Found:\n");
            printf("Name: %s\n", contacts[i].name);
            printf("Phone: %s\n", contacts[i].phone);
            printf("Email: %s\n", contacts[i].email);
            char confirm[3];
            get_valid_input("Are you sure you want to delete this contact? [y/n]: ", confirm, sizeof(confirm), CONFIRM_REGEX);
            if (confirm[0] == 'y' || confirm[0] == 'Y')
            {
                // shift left
                for (int j = i; j < contact_count - 1; j++)
                {
                    contacts[j] = contacts[j + 1];
                }

                // clear last slot
                memset(&contacts[contact_count - 1], 0, sizeof(Contact));

                contact_count--;
                found = 1;
                printf("Contact '%s' deleted Successfully\n", name);
                break;
            }
            else
            {
                printf("❌ Deletion cancelled.\n");
                found = 1;
                break;
            }
        }
    }
    if (!found)
    {
        printf("Contact '%s' not found.\n", name);
    }
}

// ----------------- Search (updated to use get_input) -----------------
void search_contact(void)
{
    if (contact_count == 0)
    {
        printf("No contacts in Contact manager, add yours :)\n");
        return;
    }

    char name[MAX_NAME_LENGTH];
    get_valid_input("Enter name: ", name, MAX_NAME_LENGTH, NAME_REGEX); // strict validation

    int found = 0;

    for (int i = 0; i < contact_count; i++)
    {
        if (strcasecmp(contacts[i].name, name) == 0)
        {
            printf("\n📞 Contact Found:\n");
            printf("Name: %s\n", contacts[i].name);
            printf("Phone: %s\n", contacts[i].phone);
            printf("Email: %s\n\n", contacts[i].email);
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("Contact not found :(\n");
    }
}

// Merge sort to sort the contacts with dynamic memory management
void merge(Contact arr[], int left, int mid, int right, SortField field)
{
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Allocate memory dynamically
    Contact *L = malloc(n1 * sizeof(Contact));
    Contact *R = malloc(n2 * sizeof(Contact));

    if (!L || !R) {
        fprintf(stderr, "Memory allocation failed in merge.\n");
        return;
    }

    // Copy data to temporary arrays
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];

    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2)
    {
        int cmp;
        if (field == SORT_BY_NAME)
            cmp = strcasecmp(L[i].name, R[j].name);
        else if (field == SORT_BY_PHONE)
            cmp = strcmp(L[i].phone, R[j].phone);
        else
            cmp = strcasecmp(L[i].email, R[j].email);

        if (cmp <= 0)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    // Copy remaining elements
    while (i < n1)
        arr[k++] = L[i++];
    while (j < n2)
        arr[k++] = R[j++];

    // Free allocated memory
    free(L);
    free(R);
}

void merge_sort(Contact arr[], int left, int right, SortField field)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;

        merge_sort(arr, left, mid, field);
        merge_sort(arr, mid + 1, right, field);
        merge(arr, left, mid, right, field);
    }
}

// Sort contacts
void sort_contacts(void)
{
    if (contact_count == 0)
    {
        printf("No contacts to sort.\n");
        return;
    }

    printf("Sort by:\n");
    printf("1. Name\n");
    printf("2. Phone\n");
    printf("3. Email\n");

    int choice = get_choice("Enter choice (1-3): ", 1, 3);

    SortField field;
    switch (choice)
    {
        case 1: field = SORT_BY_NAME; break;
        case 2: field = SORT_BY_PHONE; break;
        case 3: field = SORT_BY_EMAIL; break;
        default:
            // This shouldn't trigger because regex blocks invalid entries
            printf("Invalid choice.\n");
            return;
    }

    merge_sort(contacts, 0, contact_count - 1, field);
    printf("Contacts sorted successfully!\n");
}
