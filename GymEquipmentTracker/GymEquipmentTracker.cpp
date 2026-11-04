#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

// Equipment class
class Equipment {
protected:
	std::string name;
	std::string category;
	bool isAvailable

public:
	Equipment(std::string n, std::string cat) : name(n), category(cat), isAvailable(true) {}
	virtual ~Equipment() = default;
	// Getters
	std::string getName() const {
		return name;
	}

	std::string getCategory() const {
		return category;
	}

	bool getAvailability() const {
		return isAvailable;
	}

	void setAvailability(bool s) {
		isAvailable = s;
	}
	// Pure Virtual Function
	virtual void displayInfo() const = 0;
};

// Derived class 1
class EquipmentCategory : virtual public Equipment {
public:
	EquipmentCategory(std::string n, std::string cat)
		: Equipment(n, cat) {}

	// Override
	void displayInfo() const override {
		std::cout << "[Category] Item: " << name
				  << " | Type: " << category
				  << " | Status: " << (isAvailable ? "Available" : "In Use") << "\n";
	}
};

// Derived class 2
class Status : public Equipment {
public:
	Status(std::string n, std::string cat)
		: Equipment(n, cat) {}

	// Override
	void displayInfo() const override {
		std::cout << "[Status] Item: " << name
				  << " | Status: " << (isAvailable ? "Available" : "In Use")
				  << " | Group: " << category << "\n";
	}
};
// User class
class User {
private:
	std::string username;
public:
	User(std::string u) : username(u) {}
	std::string getUsername() const {
		return username;
	}
};

// Gym class
class Gym {
private:
	// array to store gym equipment
	std::vector<std::unique_ptr<Equipment>> inventory;
public:
	void addEquipment(std::unique_ptr<Equipment> item) {
		inventory.push_back(std:move(item));
	}

	void checkList() const {
		std::cout << "\n--- Gym Equipment List ---\n";
		for (const auto& item: inventory) {
			item->displayInfo();
		}
		std::cout << "-----------------------------\n";
	}
	void takeItem(const std::string& itemName) {
		bool found = false;
		for (auto& item : inventory) {
			if (item->getName() == itemName) {
				found = true;

				if (!item->getAvailability()) {
					throw std::runtime_error("Equipment is currently unavailable.");
				}

				item->setAvailability(false);
				std::cout << "\n>>> Success: You have taken the " << itemName << ".\n";
				return;
			}
		}
	}
	// not on list
	if (!found) {
		throw std::out_of_range("Equipment name doesn't exist in list.");
		}
	}
};

int main() {
	Gym localGym;

	// Something here

	std::cout << "==== Gym Equipment Tracker App ====";

	bool appRunning = true;
	// loop for app
	while(appRunning) {
		std::cout << "\nMain Menu:\n";
		std::cout << "1. Check Equipment List\n";
		std::cout << "2. Take Equipment Item\n";
		std::cout << "3. Exit Application\n";
		std::cout << "Select option (1-3)";

		// choices for app
		int choice;
		if (!(std::cin >> choice)) {
			std::cin.clear();
			std::cin.ignore(10000, '\n');
			std::cout << "\nInvalid choice type. Try again and enter a choice number.";
			continue;
		std::cin.ignore();

		if (choice == 1) {
			localGym.checkList();
		}
		else if (choice == 2) {
			std::cout << "Enter the exact equipment name to take: ";
			std::string targetEquipment;
			std::getline(std::cin, targetEquipment);

			try {
				localGym.takeItem(targetEquipment);
			}
			catch (constant std::exception& error) {
				std::cerr << "\n[Notification] " << error.what() << "\n";
				std::cout << "[System Recovery] Operation blocked safely. Returning to main menu.\n";
			}
		}
		else if (choice == 3) {
			std::cout << "\nExiting Gym Equipment Tracker App\n";
			appRunning = false;
		else {
			std::cout << "\nInvalid menu option. Please select 1, 2, or 3.\n";
		}

	return 0;
}
// add an array to store gym equipment data first

// something along the lines of function takeItem

// accept user input, make sure case is exact, and then check if the item-
// is available in the gym equipment array. If it is, mark it as taken and update the status. 
// If not, inform the user that the item is not available.
// test does this interfere with your work (if it does sry)
