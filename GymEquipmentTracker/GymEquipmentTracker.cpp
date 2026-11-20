#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

// case insensitive search helper
static std::string toLower(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return s;
}

// Equipment class
class Equipment {
protected:
	std::string name;
	std::string category;
	bool isAvailable;

public:
	// coutner
	static inline int liveCount = 0;
	static inline std::string lastDestroyed;

	Equipment(std::string n, std::string cat) : name(n), category(cat), isAvailable(true) { ++liveCount; }

	// heres an overload )
	Equipment(std::string n) : Equipment(n, "General") {}

	// bluh
	Equipment(const Equipment&) = delete;
	Equipment& operator=(const Equipment&) = delete;

	// what this do
	virtual ~Equipment() {
		--liveCount;
		lastDestroyed = name;
	}

	std::string getName() const { return name; }
	std::string getCategory() const { return category; }
	bool getAvailability() const { return isAvailable; }
	void setAvailability(bool s) { isAvailable = s; }

	virtual std::string displayInfo() const = 0;
};

// Derived class 1
class EquipmentCategory : public Equipment {
public:
	EquipmentCategory(std::string n, std::string cat) : Equipment(n, cat) {}
	EquipmentCategory(std::string n) : Equipment(n) {}

	std::string displayInfo() const override {
		return "[Category] Item: " + name + " | Type: " + category +
			" | Status: " + (isAvailable ? "Available" : "In Use");
	}
};

// Derived class 2
class Status : public Equipment {
public:
	Status(std::string n, std::string cat) : Equipment(n, cat) {}
	Status(std::string n) : Equipment(n) {}

	std::string displayInfo() const override {
		return "[Status] Item: " + name + " | Status: " + (isAvailable ? "Available" : "In Use") +
			" | Group: " + category;
	}
};

// User class
class User {
private:
	std::string username;
public:
	User(std::string u) : username(u) {}
	std::string getUsername() const { return username; }
};

// Gym class
class Gym {
private:
	std::vector<std::unique_ptr<Equipment>> inventory;

	// helper: finds index by name, throws if missing
	size_t indexOf(const std::string& itemName) const {
		for (size_t i = 0; i < inventory.size(); ++i) {
			if (inventory[i]->getName() == itemName) return i;
		}
		throw std::out_of_range("Equipment name doesn't exist in list.");
	}

	bool hasName(const std::string& itemName) const {
		for (const auto& item : inventory) {
			if (item->getName() == itemName) return true;
		}
		return false;
	}

public:
	void addEquipment(std::unique_ptr<Equipment> item) {
		inventory.push_back(std::move(item));
	}

	
	void addNewEquipment(const std::string& itemName, std::string itemCategory) {
		if (itemName.empty()) {
			throw std::invalid_argument("Type a name for the new equipment item.");
		}

		if (itemName.find_first_of("|\r\n") != std::string::npos ||
			itemCategory.find_first_of("|\r\n") != std::string::npos) {
			throw std::invalid_argument("Names and categories can't contain '|' or line breaks.");
		}
		if (hasName(itemName)) {
			throw std::invalid_argument("Equipment \"" + itemName + "\" already exists.");
		}
		if (itemCategory.empty()) itemCategory = "General";
		addEquipment(std::make_unique<EquipmentCategory>(itemName, itemCategory));
	}

	size_t size() const { return inventory.size(); }

	const Equipment& at(size_t i) const { return *inventory.at(i); }

	void takeItem(size_t index) {
		if (index >= inventory.size()) {
			throw std::out_of_range("Equipment index doesn't exist in list.");
		}
		Equipment& item = *inventory[index];
		if (!item.getAvailability()) {
			throw std::runtime_error("Equipment is currently unavailable.");
		}
		item.setAvailability(false);
	}

	void takeItem(const std::string& itemName) {
		takeItem(indexOf(itemName));
	}

	void returnItem(size_t index) {
		if (index >= inventory.size()) {
			throw std::out_of_range("Equipment index doesn't exist in list.");
		}
		Equipment& item = *inventory[index];
		if (item.getAvailability()) {
			throw std::runtime_error("Equipment is already in the gym equipment list.");
		}
		item.setAvailability(true);
	}

	void returnItem(const std::string& itemName) {
		returnItem(indexOf(itemName));
	}

	void removeItem(size_t index) {
		if (index >= inventory.size()) {
			throw std::out_of_range("Equipment index doesn't exist in list.");
		}
		if (!inventory[index]->getAvailability()) {
			throw std::runtime_error("Equipment is currently in use. Return it before removing it.");
		}
		inventory.erase(inventory.begin() + static_cast<std::ptrdiff_t>(index));
	}

	void removeItem(const std::string& itemName) {
		removeItem(indexOf(itemName));
	}

	void saveToFile(const std::string& filename) const {
		std::ofstream out(filename);
		if (!out) {
			throw std::runtime_error("Could not open file for saving: " + filename);
		}
		for (const auto& item : inventory) {
			std::string typeTag = (dynamic_cast<const EquipmentCategory*>(item.get()) != nullptr) ? "Category" : "Status";
			out << typeTag << "|"
				<< item->getName() << "|"
				<< item->getCategory() << "|"
				<< (item->getAvailability() ? "1" : "0") << "\n";
		}
	}

	void loadFromFile(const std::string& filename) {
		std::ifstream in(filename);
		if (!in) {
			throw std::runtime_error("Save file not found: " + filename);
		}
		std::vector<std::unique_ptr<Equipment>> tempInventory;
		std::string line;
		while (std::getline(in, line)) {
			if (line.empty()) continue;
			std::stringstream ss(line);
			std::string typeTag;
			std::string itemName;
			std::string itemCategory;
			std::string availFlag;
			if (!std::getline(ss, typeTag, '|') ||
				!std::getline(ss, itemName, '|') ||
				!std::getline(ss, itemCategory, '|') ||
				!std::getline(ss, availFlag, '|')) {
				throw std::runtime_error("Malformed record in save file.");
			}
			if (availFlag != "0" && availFlag != "1") {
				throw std::runtime_error("Invalid availability flag in save file.");
			}

			std::unique_ptr<Equipment> eq;
			if (typeTag == "Category") {
				eq = std::make_unique<EquipmentCategory>(itemName, itemCategory);
			}
			else if (typeTag == "Status") {
				eq = std::make_unique<Status>(itemName, itemCategory);
			}
			else {
				throw std::runtime_error("Unknown equipment type in save file.");
			}

			eq->setAvailability(availFlag == "1");
			tempInventory.push_back(std::move(eq));
		}
		inventory = std::move(tempInventory);
	}

	// count available items overall...
	size_t countAvailable() const {
		size_t n = 0;
		for (const auto& item : inventory) {
			if (item->getAvailability()) ++n;
		}
		return n;
	}

	// ...or within one category
	size_t countAvailable(const std::string& category) const {
		size_t n = 0;
		for (const auto& item : inventory) {
			if (item->getCategory() == category && item->getAvailability()) ++n;
		}
		return n;
	}

	// i'd say only gog and i know how this code works but i barely know what im doing
	std::vector<size_t> search(const std::string& term) const {
		std::vector<size_t> results;
		std::string t = toLower(term);
		for (size_t i = 0; i < inventory.size(); ++i) {
			if (t.empty() ||
				toLower(inventory[i]->getName()).find(t) != std::string::npos ||
				toLower(inventory[i]->getCategory()).find(t) != std::string::npos) {
				results.push_back(i);
			}
		}
		return results;
	}

	// sdgsdgsdg
	std::vector<size_t> search(const std::string& term, const std::string& category) const {
		std::vector<size_t> results;
		std::string t = toLower(term);
		for (size_t i = 0; i < inventory.size(); ++i) {
			if (inventory[i]->getCategory() != category) continue;
			if (t.empty() || toLower(inventory[i]->getName()).find(t) != std::string::npos) {
				results.push_back(i);
			}
		}
		return results;
	}
};

// the gui stuff

class GymApp {
public:
	Gym gym;
	std::string saveFile = "gym_equipment.txt";

	Fl_Double_Window* window;
	Fl_Hold_Browser* list;
	Fl_Input* searchInput;
	Fl_Button* searchBtn;
	Fl_Button* clearBtn;
	Fl_Input* nameInput;
	Fl_Button* takeBtn;
	Fl_Button* returnBtn;
	Fl_Button* exitBtn;
	Fl_Input* categoryInput;   
	Fl_Button* addBtn;         
	Fl_Button* removeBtn;      
	Fl_Box* statusBox;
	Fl_Button* testBadFileBtn;

	// hrhfh
	std::vector<size_t> visible;
	std::string currentFilter;

	GymApp() {
		// help its 2am
		try {
			gym.loadFromFile(saveFile);
		}
		catch (const std::exception&) {
			gym.addEquipment(std::make_unique<EquipmentCategory>("Treadmill 1", "Cardio"));
			gym.addEquipment(std::make_unique<EquipmentCategory>("Treadmill 2", "Cardio"));
			gym.addEquipment(std::make_unique<EquipmentCategory>("Treadmill 3", "Cardio"));
			gym.addEquipment(std::make_unique<EquipmentCategory>("Treadmill 4", "Cardio"));
			gym.addEquipment(std::make_unique<EquipmentCategory>("Treadmill 5", "Cardio"));
			gym.addEquipment(std::make_unique<EquipmentCategory>("Treadmill 6", "Cardio"));
			gym.addEquipment(std::make_unique<EquipmentCategory>("Treadmill 7", "Cardio"));
			gym.addEquipment(std::make_unique<EquipmentCategory>("Treadmill 8", "Cardio"));
			gym.addEquipment(std::make_unique<Status>("Dumbbells 1 (10 lbs)", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Dumbbells 1 (20 lbs)", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Dumbbells 1 (40 lbs)", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Dumbbells 2 (10 lbs)", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Dumbbells 2 (20 lbs)", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Dumbbells 2 (40 lbs)", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Dumbbells 3 (10 lbs)", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Dumbbells 3 (20 lbs)", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Dumbbells 3 (40 lbs)", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Leg Press Machine 1", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Leg Press Machine 2", "Strength"));
			gym.addEquipment(std::make_unique<Status>("Leg Press Machine 3", "Strength"));
			gym.addEquipment(std::make_unique<EquipmentCategory>("Rowing Machine", "Cardio"));
			gym.saveToFile(saveFile);
		}

		buildWindow();
		refreshList();
	}

	void buildWindow() {
		window = new Fl_Double_Window(640, 610, "Gym Equipment Tracker App");

		Fl_Box* title = new Fl_Box(20, 10, 600, 30, "Gym Equipment List");
		title->labelfont(FL_BOLD);
		title->labelsize(18);

		// search row, right under the title (label sits to the left of the box)
		searchInput = new Fl_Input(90, 48, 320, 30, "Search:");
		searchInput->when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);
		searchInput->callback(onSearch, this);   // pressing Enter searches

		searchBtn = new Fl_Button(420, 48, 95, 30, "Search");
		searchBtn->callback(onSearch, this);

		clearBtn = new Fl_Button(525, 48, 95, 30, "Clear");
		clearBtn->callback(onClear, this);

		list = new Fl_Hold_Browser(20, 90, 600, 200);
		list->callback(onSelect, this);
		list->textsize(14);

		// also used as the "new item name" field when adding
		nameInput = new Fl_Input(20, 325, 600, 30, "Equipment name (click a row or type exact name):");
		nameInput->align(FL_ALIGN_TOP_LEFT);

		takeBtn = new Fl_Button(20, 375, 190, 40, "Take Equipment Item");
		takeBtn->callback(onTake, this);

		returnBtn = new Fl_Button(225, 375, 190, 40, "Return Equipment Item");
		returnBtn->callback(onReturn, this);

		exitBtn = new Fl_Button(430, 375, 190, 40, "Exit Application");
		exitBtn->callback(onExit, this);

		// NEW: add / remove row
		categoryInput = new Fl_Input(20, 450, 300, 30, "Category for new item (blank = General):");
		categoryInput->align(FL_ALIGN_TOP_LEFT);

		addBtn = new Fl_Button(335, 440, 135, 40, "Add Item");
		addBtn->callback(onAdd, this);

		removeBtn = new Fl_Button(485, 440, 135, 40, "Remove Item");
		removeBtn->callback(onRemove, this);

		statusBox = new Fl_Box(20, 495, 600, 60, "Welcome! Select an item to take or return.");
		statusBox->box(FL_DOWN_BOX);
		statusBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_WRAP);

		testBadFileBtn = new Fl_Button(20, 565, 600, 30, "Test Corrupted File Load (bad_data.txt)");
		testBadFileBtn->callback(onLoadCorrupted, this);

		window->end();
		window->callback(onExit, this);
	}

	// rebuilds browser from inventory
	void refreshList() {
		list->clear();
		visible = gym.search(currentFilter);
		for (size_t idx : visible) {
			const Equipment& item = gym.at(idx);
			int color = item.getAvailability() ? FL_DARK_GREEN : FL_RED;
			std::string line = "@C" + std::to_string(color) + "@." + item.displayInfo();
			list->add(line.c_str());
		}
	}

	void setStatus(const std::string& msg, bool isError) {
		statusBox->copy_label(msg.c_str());
		statusBox->labelcolor(isError ? FL_RED : FL_DARK_GREEN);
		statusBox->redraw();
	}

	void setStatus(const std::string& msg) {
		setStatus(msg, false);
	}

	void setStatus(const std::string& msg, const std::string& category) {
		setStatus(msg + "  [" + category + " available: " +
			std::to_string(gym.countAvailable(category)) + "]", false);
	}

	size_t selectedIndex() const {
		int row = list->value();
		if (row <= 0 || static_cast<size_t>(row) > visible.size()) {
			throw std::invalid_argument("Select a row or type an equipment name.");
		}
		return visible[static_cast<size_t>(row - 1)];
	}

	static void onSelect(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		int row = app->list->value();
		if (row > 0 && static_cast<size_t>(row) <= app->visible.size()) {
			app->nameInput->value(app->gym.at(app->visible[row - 1]).getName().c_str());
		}
	}

	static void onSearch(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		app->currentFilter = app->searchInput->value();
		app->refreshList();

		size_t found = app->visible.size();
		if (app->currentFilter.empty()) {
			app->setStatus("Showing all " + std::to_string(found) + " items.");
		}
		else if (found == 0) {
			app->setStatus("No equipment found matching \"" + app->currentFilter + "\".", true);
		}
		else {
			app->setStatus("Found " + std::to_string(found) + " item(s) matching \"" +
				app->currentFilter + "\".");
		}
	}

	static void onClear(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		app->searchInput->value("");
		app->currentFilter.clear();
		app->refreshList();
		app->setStatus("Search cleared. Showing all " + std::to_string(app->visible.size()) + " items.");
	}

	static void onTake(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		std::string target = app->nameInput->value();
		try {
			std::string category;
			if (!target.empty()) {
				app->gym.takeItem(target);                       
				category = app->categoryOf(target);
			}
			else {
				size_t idx = app->selectedIndex();
				app->gym.takeItem(idx);                          
				target = app->gym.at(idx).getName();
				category = app->gym.at(idx).getCategory();
			}
			app->gym.saveToFile(app->saveFile);
			app->setStatus("Success: You have taken the " + target + ".", category);
		}
		catch (const std::exception& error) {
			app->setStatus(std::string("[Notification] ") + error.what(), true);
			fl_alert("%s", error.what());
		}
		app->refreshList();
	}

	static void onReturn(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		std::string target = app->nameInput->value();
		try {
			std::string category;
			if (!target.empty()) {
				app->gym.returnItem(target);                     
				category = app->categoryOf(target);
			}
			else {
				size_t idx = app->selectedIndex();
				app->gym.returnItem(idx);                        
				target = app->gym.at(idx).getName();
				category = app->gym.at(idx).getCategory();
			}
			app->gym.saveToFile(app->saveFile);
			app->setStatus("Success: You have returned the " + target + ".", category);
		}
		catch (const std::exception& error) {
			app->setStatus(std::string("[Notification] ") + error.what(), true);
			fl_alert("%s", error.what());
		}
		app->refreshList();
	}

	static void onAdd(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		std::string name = app->nameInput->value();
		std::string category = app->categoryInput->value();
		try {
			app->gym.addNewEquipment(name, category);
			app->gym.saveToFile(app->saveFile);

			app->searchInput->value("");
			app->currentFilter.clear();
			app->nameInput->value("");
			app->categoryInput->value("");

			app->setStatus("Success: Added \"" + name + "\".  [Equipment objects alive: " +
				std::to_string(Equipment::liveCount) + "]");
		}
		catch (const std::exception& error) {
			app->setStatus(std::string("[Notification] ") + error.what(), true);
			fl_alert("%s", error.what());
		}
		app->refreshList();
	}

	static void onRemove(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		std::string target = app->nameInput->value();
		try {
			if (!target.empty()) {
				app->gym.removeItem(target);                     
			}
			else {
				size_t idx = app->selectedIndex();
				target = app->gym.at(idx).getName();             
				app->gym.removeItem(idx);                        
			}
			app->gym.saveToFile(app->saveFile);
			app->nameInput->value("");
			app->setStatus("Success: Removed \"" + Equipment::lastDestroyed +
				"\" (destructor ran).  [Equipment objects alive: " +
				std::to_string(Equipment::liveCount) + "]");
		}
		catch (const std::exception& error) {
			app->setStatus(std::string("[Notification] ") + error.what(), true);
			fl_alert("%s", error.what());
		}
		app->refreshList();
	}

	static void onExit(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		app->window->hide();
	}

	static void onLoadCorrupted(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		try {
			app->gym.loadFromFile("bad_data.txt");
			app->setStatus("Loaded bad_data.txt", false);
		}
		catch (const std::exception& error) {
			app->setStatus(std::string("[File Error] ") + error.what(), true);
			fl_alert("File Error: %s\nValid inventory preserved.", error.what());
		}
		app->refreshList();
	}
private:
	// search category
	std::string categoryOf(const std::string& itemName) const {
		for (size_t i = 0; i < gym.size(); ++i) {
			if (gym.at(i).getName() == itemName) return gym.at(i).getCategory();
		}
		return "General";
	}
};

int main(int argc, char** argv) {
	GymApp app;
	app.window->show(argc, argv);
	return Fl::run();
}
	GymApp app;
	app.window->show(argc, argv);
	return Fl::run();
}
