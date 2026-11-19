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

// case sensitive search stuff
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
	Equipment(std::string n, std::string cat) : name(n), category(cat), isAvailable(true) {}

	// heres an overload
	Equipment(std::string n) : Equipment(n, "General") {}

	virtual ~Equipment() = default;

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

public:
	void addEquipment(std::unique_ptr<Equipment> item) {
		inventory.push_back(std::move(item));
	}

	size_t size() const { return inventory.size(); }

	const Equipment& at(size_t i) const { return *inventory.at(i); }

	//take by index (used with the selected browser row)
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

	//take by name (typed into the input box)
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

	// Persistence
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
			if(!std::getline(ss, typeTag, '|') ||
				    !std::getline(ss, itemName, '|') ||
				    !std::getline(ss, itemCategory, '|') ||
				    !std::getline(ss, availFlag, '|')) {
				    throw std::runtime_error("Malformed record in save file.");
			}
			std::unique_ptr<Equipment> eq = (typeTag == "Category")
					? std::unique_ptr<Equipment>(std::make_unique<EquipmentCategory>(itemName, itemCategory))
					: std::unique_ptr<Equipment>(std::make_unique<Status>(itemName, itemCategory));
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

	// search list
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

	// ...or restrict the search to one exact category (name match only)
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
	Fl_Box* statusBox;
	Fl_Button* testBadFileBtn;

	// maps browser row (row - 1) -> inventory index, so filtering doesn't break selection
	std::vector<size_t> visible;
	std::string currentFilter;

	GymApp() {
		// Inventory
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
		window = new Fl_Double_Window(640, 495, "Gym Equipment Tracker App");

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

		nameInput = new Fl_Input(20, 325, 600, 30, "Equipment name (click a row or type exact name):");
		nameInput->align(FL_ALIGN_TOP_LEFT);

		takeBtn = new Fl_Button(20, 375, 190, 40, "Take Equipment Item");
		takeBtn->callback(onTake, this);

		returnBtn = new Fl_Button(225, 375, 190, 40, "Return Equipment Item");
		returnBtn->callback(onReturn, this);

		

		exitBtn = new Fl_Button(430, 375, 190, 40, "Exit Application");
		exitBtn->callback(onExit, this);

		statusBox = new Fl_Box(20, 430, 600, 50, "Welcome! Select an item to take or return.");
		statusBox->box(FL_DOWN_BOX);
		statusBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_WRAP);

		window->end();
		window->callback(onExit, this);
	}

	// rebuilds browser from inventory, showing only items matching currentFilter
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

	// explicit error flag
	void setStatus(const std::string& msg, bool isError) {
		statusBox->copy_label(msg.c_str());
		statusBox->labelcolor(isError ? FL_RED : FL_DARK_GREEN);
		statusBox->redraw();
	}

	// success message (no flag needed)
	void setStatus(const std::string& msg) {
		setStatus(msg, false);
	}

	// message plus a live "available" count for a category
	void setStatus(const std::string& msg, const std::string& category) {
		setStatus(msg + "  [" + category + " available: " +
			std::to_string(gym.countAvailable(category)) + "]", false);
	}

	// returns the inventory index of the selected row, or throws if nothing selected
	size_t selectedIndex() const {
		int row = list->value();
		if (row <= 0 || static_cast<size_t>(row) > visible.size()) {
			throw std::invalid_argument("Select a row or type an equipment name.");
		}
		return visible[static_cast<size_t>(row - 1)];
	}

	// callbacks
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
				app->gym.takeItem(target);                       // by name
				category = app->categoryOf(target);
			}
			else {
				size_t idx = app->selectedIndex();
				app->gym.takeItem(idx);                          // by index
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
				app->gym.returnItem(target);                     // by name
				category = app->categoryOf(target);
			}
			else {
				size_t idx = app->selectedIndex();
				app->gym.returnItem(idx);                        // by index
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

	static void onExit(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		app->window->hide();
	}

private:
	// helper: look up an item's category by name
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
