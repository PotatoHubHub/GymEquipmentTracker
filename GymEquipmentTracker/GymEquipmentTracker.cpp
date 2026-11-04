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
	if (!found) {
		throw std::out_of_range("Equipment name doesn't exist in list.");
		}
	}
};

int main() {
	Gym localGym;

	return 0;
}
// add an array to store gym equipment data first

// something along the lines of function takeItem

// accept user input, make sure case is exact, and then check if the item-
// is available in the gym equipment array. If it is, mark it as taken and update the status. 
// If not, inform the user that the item is not available.
// test does this interfere with your work (if it does sry)
