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

    cli.addPositionalArgument("FONT_FILE|FONT_FAMILY", "Either the path to a font file, or a font family name.", "[FONT_FILE|FONT_FAMILY...]");

    // TODO: This crashes when calling from MSYS with globs that expand to something containing e.g. Chinese characters. For example,
    // the following will crash if the current directory contains TTF files with Chinese characters:
    //
    //		fontpreview *.ttf
    //
    // The problem is related to: https://bugreports.qt.io/browse/QTBUG-67515
    cli.process(app);

    QStringList posArgs = cli.positionalArguments();

    app.setOrganizationName("alemariusnexus");
    app.setOrganizationDomain("alemariusnexus.com");
    app.setApplicationName("fontpreview");

    MainWindow* win = new MainWindow;

    // TODO: This does not open/use fonts in the same order as passed to command line, because openFontFiles() happens at the end,
    // giving font files priority over font families.
    QStringList fontFiles;

    for (QString fontArg : posArgs) {
    	if (QFileInfo(fontArg).isFile()) {
    		fontFiles.append(fontArg);
    	} else {
    		win->useFontFamily(fontArg);
    	}
    }

    win->openFontFiles(fontFiles);

    win->show();
    
    return app.exec();
}
