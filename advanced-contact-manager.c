#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   // for strcasecmp

#define MAX_CONTACTS 100
#define MAX_NAME_LENGTH 50
#define MAX_PHONE_LENGTH 15
#define MAX_EMAIL_LENGTH 25

#define NAME_REGEX "^[A-Za4-z][A-Za-z '-]{0,48}[A-Za-z]$"
#define EMAIL_REGEX "^[a-z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"
#define PHONE_REGEX "^[0-9]{10}$"

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

// New helper prototypes
void get_input(const char *prompt, char *buffer, size_t size);
void get_valid_input(const char *prompt, char *buffer, size_t size, const char *pattern, int allow_empty);

typedef struct
{
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    char email[MAX_EMAIL_LENGTH];
} Contact;

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
                printf("Exiting the program. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    while (choice != 6);
    save_contacts();
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
void save_contacts()
{
    FILE *file = fopen("contacts.txt", "w");
    if (file == NULL)
    {
        printf("❌ Error opening file to save contacts.\n");
        return;
    }

    for (int i = 0; i < contact_count; i++)
    {
        fprintf(file, "%s, %s, %s\n", contacts[i].name, contacts[i].phone, contacts[i].email);
    }
    fclose(file);
    printf("✅ Contacts saved successfully to file!\n");
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

    while (fscanf(file, "%49[^,], %19[^,], %49[^\n]\n",
                  contacts[contact_count].name,
                  contacts[contact_count].phone,
                  contacts[contact_count].email) == 3)
    {
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
    printf("6. Exit\n");
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
            if (choice < 1 || choice > 6)
            {
                printf("Choice out of range. Please enter 1-6.\n");
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
}

void get_valid_input(const char *prompt, char *buffer, size_t size, const char *pattern, int allow_empty)
{
    while (1)
    {
        get_input(prompt, buffer, size);

        if (buffer[0] == '\0')
        {
            if (allow_empty)
                return; // Keep existing value
            else
            {
                printf("Input cannot be empty. Please try again.\n");
                continue;
            }
        }

        if (pattern == NULL || validate_with_regex(pattern, buffer))
        {
            return; // valid
        }
        else
        {
            if (allow_empty)
                printf("Invalid format. Please try again (or press Enter to keep existing).\n");
            else
                printf("Invalid format. Please try again.\n");
        }
    }
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
    get_valid_input("Enter name: ", contacts[contact_count].name, MAX_NAME_LENGTH, NAME_REGEX, 0);
    get_valid_input("Enter phone (10 digits): ", contacts[contact_count].phone, MAX_PHONE_LENGTH, PHONE_REGEX, 0);
    get_valid_input("Enter email: ", contacts[contact_count].email, MAX_EMAIL_LENGTH, EMAIL_REGEX, 0);

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
        printf("No Contacts found\n");
        return;
    }

    printf("\n📋 All Contacts:\n");

    for (int i = 0; i < contact_count; i++)
    {
        printf("\n Contact %i:\n", i + 1);
        printf("Name: %s\n", contacts[i].name);
        printf("Phone: %s\n", contacts[i].phone);
        printf("Email: %s\n", contacts[i].email);
    }
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
    get_input("Enter name to update: ", name, MAX_NAME_LENGTH, 0);

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

            char tmp[MAX_EMAIL_LENGTH]; // large enough for any field

            // Name (optional)
            get_valid_input("Enter new name: ", tmp, MAX_NAME_LENGTH, NAME_REGEX, 1);
            if (tmp[0] != '\0') strcpy(contacts[i].name, tmp);

            // Phone (optional + validated)
            get_valid_input("Enter new phone (10 digits): ", tmp, MAX_PHONE_LENGTH, PHONE_REGEX, 1);
            if (tmp[0] != '\0') strcpy(contacts[i].phone, tmp);

            // Email (optional + validated)
            get_valid_input("Enter new email: ", tmp, MAX_EMAIL_LENGTH, EMAIL_REGEX, 1);
            if (tmp[0] != '\0') strcpy(contacts[i].email, tmp);

            printf("\n✅ Contact updated successfully!\n");
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
    get_input("Enter name to delete: ", name, MAX_NAME_LENGTH);

    int found = 0;

    for (int i = 0; i < contact_count; i++)
    {
        if (strcasecmp(contacts[i].name, name) == 0)
        {
            // shift left
            for (int j = i; j < contact_count - 1; j++)
            {
                contacts[j] = contacts[j + 1];
            }

            contact_count--;
            found = 1;
            printf("Contact '%s' deleted Successfully\n", name);
            break;
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
    get_valid_input("Enter name: ", name, MAX_NAME_LENGTH, NAME_REGEX, 0); // strict validation

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
