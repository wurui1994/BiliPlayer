#include "Application.h"
#include "Player.h"
#include "Prefer.h"

#pragma comment(lib,"mpv")

int main(int argc, char *argv[])
{
	Application app(argc, argv);
	//
	app.setAttribute(Qt::AA_EnableHighDpiScaling);
	//
	Player player;
	// parse command line
	QStringList args = QApplication::arguments();
	if (args.size() >= 2)
	{
		player.Load(args.at(1));
	}	
	else
	{
		player.Load();
	}
	//
	player.show();
	//
	return app.exec();
}