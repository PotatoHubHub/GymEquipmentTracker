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

// gog help ye all who enter here

// Equipment class
class Equipment {
protected:
	std::string name;
	std::string category;
	bool isAvailable;

public:
	Equipment(std::string n, std::string cat) : name(n), category(cat), isAvailable(true) {}

	// overload here 
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
	EquipmentCategory(std::string n) : Equipment(n) {}   // heres one (overload)

	std::string displayInfo() const override {
		return "[Category] Item: " + name + " | Type: " + category +
			" | Status: " + (isAvailable ? "Available" : "In Use");
	}
};

// Derived class 2
class Status : public Equipment {
public:
	Status(std::string n, std::string cat) : Equipment(n, cat) {}
	Status(std::string n) : Equipment(n) {}              // (another overload)

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

	// some more various overload usage
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

	// overload usage once again
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

	// overload to count items
	size_t countAvailable() const {
		size_t n = 0;
		for (const auto& item : inventory) {
			if (item->getAvailability()) ++n;
		}
		return n;
	}

	// also for counting within a category
	size_t countAvailable(const std::string& category) const {
		size_t n = 0;
		for (const auto& item : inventory) {
			if (item->getCategory() == category && item->getAvailability()) ++n;
		}
		return n;
	}
};

// the gui stuff

class GymApp {
public:
	Gym gym;

	Fl_Double_Window* window;
	Fl_Hold_Browser* list;
	Fl_Input* nameInput;
	Fl_Button* takeBtn;
	Fl_Button* returnBtn;
	Fl_Button* exitBtn;
	Fl_Box* statusBox;

	GymApp() {
		// Inventory
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

		buildWindow();
		refreshList();
	}

	void buildWindow() {
		window = new Fl_Double_Window(640, 470, "Gym Equipment Tracker App");

		Fl_Box* title = new Fl_Box(20, 10, 600, 30, "Gym Equipment List");
		title->labelfont(FL_BOLD);
		title->labelsize(18);

		list = new Fl_Hold_Browser(20, 50, 600, 230);
		list->callback(onSelect, this);
		list->textsize(14);

		nameInput = new Fl_Input(20, 305, 600, 30, "Equipment name (click a row or type exact name):");
		nameInput->align(FL_ALIGN_TOP_LEFT);

		takeBtn = new Fl_Button(20, 350, 190, 40, "Take Equipment Item");
		takeBtn->callback(onTake, this);

		returnBtn = new Fl_Button(225, 350, 190, 40, "Return Equipment Item");
		returnBtn->callback(onReturn, this);

		exitBtn = new Fl_Button(430, 350, 190, 40, "Exit Application");
		exitBtn->callback(onExit, this);

		statusBox = new Fl_Box(20, 405, 600, 50, "Welcome! Select an item to take or return.");
		statusBox->box(FL_DOWN_BOX);
		statusBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_WRAP);

		window->end();
		window->callback(onExit, this);
	}

	// rebuilds browser from inventory
	void refreshList() {
		list->clear();
		for (size_t i = 0; i < gym.size(); ++i) {
			const Equipment& item = gym.at(i);
			int color = item.getAvailability() ? FL_DARK_GREEN : FL_RED;
			std::string line = "@C" + std::to_string(color) + "@." + item.displayInfo();
			list->add(line.c_str());
		}
	}

	// overload error flag
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

	static void onSelect(Fl_Widget*, void* data) {
		GymApp* app = static_cast<GymApp*>(data);
		int row = app->list->value();
		if (row > 0) {
			app->nameInput->value(app->gym.at(row - 1).getName().c_str());
		}
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
				int row = app->list->value();
				if (row <= 0) throw std::invalid_argument("Select a row or type an equipment name.");
				size_t idx = static_cast<size_t>(row - 1);
				app->gym.takeItem(idx);                          // by index
				target = app->gym.at(idx).getName();
				category = app->gym.at(idx).getCategory();
			}
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
				int row = app->list->value();
				if (row <= 0) throw std::invalid_argument("Select a row or type an equipment name.");
				size_t idx = static_cast<size_t>(row - 1);
				app->gym.returnItem(idx);                        // by index
				target = app->gym.at(idx).getName();
				category = app->gym.at(idx).getCategory();
			}
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
