### 📱 Contact Management System (C)

A console-based Contact Management System implemented in C, designed to manage a collection of contacts with robust input validation, file persistence, and vCard integration.

---

### ✨ Features

### 🆕 Add Contact

Prompts for Name, Phone, and Email.

Validates input using regex:

Name → letters, spaces, hyphens, apostrophes (1–49 chars).

Phone → Indian & International formats.

Email → standard email pattern.

Sanitizes inputs (trims whitespace, removes commas).

Adds contact if valid and space is available.

### 👀 View Contacts

Displays all contacts in a formatted table:
Index | Name | Phone | Email.

### 🔍 Search Contacts

Supports exact or partial name search (case-insensitive).

Displays results in a table format.

### ✏️ Update Contact

Select a contact by name.

Update Name, Phone, or Email individually.

Empty input retains old value.

Validations applied on new values.

### 🗑️ Delete Contact

Deletes contact by name with confirmation (Y/N).

Shifts array to maintain order after deletion.

### 🔀 Sort Contacts

Sorts by Name, Phone, or Email using Merge Sort (O(n log n)).

### 💾 File Persistence

Saves contacts to contacts.txt in CSV format.

Loads existing contacts at startup.

Handles max contact limit gracefully.

### 📇 vCard (VCF) Integration

Export: writes contacts to contacts.vcf (vCard 3.0), one phone (CELL) and one email (WORK) per contact.

Import: reads multiple VCARDs from a file, stores last TEL/EMAIL, stops at max contacts.

### ✅ Input Validation & Sanitization

Regex-based validation for Name, Phone, Email, and menu choices.

Sanitizes inputs to prevent CSV/VCF formatting issues.

### 🖥️ User Interface

Menu-driven system with emoji feedback for actions: ✅, ❌, ℹ️.

Clear prompts and error messages for invalid input.

---

### 🛠️ Installation & Compilation
#### Using CS50 IDE (recommended):

Navigate to the project directory:

```sh
cd path/to/advanced-contact-manager
```

Compile using make:

```sh
make advanced-contact-manager
```

#### Run the program:

```sh
./advanced-contact-manager
```

#### Without CS50 make:

If make is not available, you can compile manually with gcc:

```sh
gcc -o advanced-contact-manager advanced-contact-manager.c -Wall -std=c11
./advanced-contact-manager
```

✅ Tip: Always run ./advanced-contact-manager from the project directory.

---

### File Structure 📂
```
advanced-contact-manager/
├── advanced-contact-manager.c   # Main C source code
├── contacts.txt                 # Saved contacts (CSV)
├── contacts.vcf                 # Exported contacts (vCard)
└── README.md                    # Project documentation
```
Note: contacts.txt and contacts.vcf not added here.

---

### 🧭 Usage

Upon starting, the system displays a menu:

<img width="750" height="366" alt="image" src="https://github.com/user-attachments/assets/50a6b8df-44ee-4eba-b071-8995517e209a" />

✅ Success and ❌ error messages guide the user throughout.

ℹ️ Informational messages appear for no changes or warnings.

### 💾 Data Storage

CSV File: contacts.txt (Name, Phone, Email)

VCF File: contacts.vcf (vCard 3.0 format)

Maximum of 100 contacts stored in memory at a time.

Automatic loading at startup and saving at exit.

### 📂 File Format Details

#### CSV

Name, Phone, Email

John Doe, +919876543210, john@example.com

Jane Smith, +14155552671, jane@domain.com

#### vCard (VCF)

BEGIN:VCARD

VERSION:3.0

FN:John Doe

TEL;TYPE=CELL:+919876543210

EMAIL;TYPE=WORK:john@example.com

END:VCARD

---

### ⚙️ Technical Details

✅ Implemented in C using structs and arrays.

✅ Input validation with regex.

✅ File I/O handling ensures data persistence.

✅ Merge Sort used for efficient sorting.

✅ Case-insensitive operations via strcasecmp().

✅ Memory-safe with bounds checks and input sanitization.

⚠️ Maximum 100 contacts; additional entries ignored.

---

### 📈 Advantages

Easy-to-use menu interface.

Supports international phone numbers.

Emoji-based user-friendly feedback.

Import/export to standard vCard for cross-platform compatibility.

---

### 👨‍💻 Future Enhancements

Prevent duplicate contacts during vCard import.

Add multiple phone numbers/emails per contact.

Implement search by phone or email.

Integrate graphical UI for better experience.

---

### 📝 License

This project is licensed under the MIT License. See the LICENSE file for details.
