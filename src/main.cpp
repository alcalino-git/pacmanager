#include <stdio.h>
#include <gtkmm.h>
#include "./search.hpp"


class DefaultWindow : public Gtk::Window{
	SearchComponent search_component;

	public:
	DefaultWindow() {
		set_title("Pacmanager");
		set_size_request(800, 500);

		this->set_child(search_component);
	}
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("org.gtkmm.pacmanager");

  return app->make_window_and_run<DefaultWindow>(argc, argv);
}
