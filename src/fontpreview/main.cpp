#include <cstdio>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include "MainWindow.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    
    QCommandLineParser cli;
    cli.setApplicationDescription("A simple font previewer");
    cli.addHelpOption();
    cli.addVersionOption();

    cli.addPositionalArgument("FONT_FILE|FONT_FAMILY", "Either the path to a font file, or a font family name.");

    cli.process(app);

    QStringList posArgs = cli.positionalArguments();

    app.setOrganizationName("alemariusnexus");
    app.setOrganizationDomain("alemariusnexus.com");
    app.setApplicationName("fontpreview");

    MainWindow* win = new MainWindow;

    if (!posArgs.empty()) {
    	QString fontArg = posArgs[0];

    	if (QFileInfo(fontArg).isFile()) {
    		win->openFontFile(fontArg);
    	} else {
    		win->useFontFamily(fontArg);
    	}
    }

    win->show();
    
    return app.exec();
}
