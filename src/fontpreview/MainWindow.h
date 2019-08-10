#ifndef FONTPREVIEW_MAINWINDOW_H_
#define FONTPREVIEW_MAINWINDOW_H_

#include <QMainWindow>
#include <QListWidget>
#include <QList>
#include <ui_MainWindow.h>

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	struct LoadedFontFile
	{
		QString fontFile;
		QStringList fontFamilies;
	};

public:
	MainWindow();

	void openFontFile(QString fontFile);
	void openFontFiles(QStringList fontFiles);
	void useFontFamily(const QString& family);

protected:
	void closeEvent(QCloseEvent* evt);

private slots:
	void loadGeometryFromSettings();

	void sampleTextChanged();

	void previewTextTemplateListItemActivated(QListWidgetItem* cur);
	void previewTextTemplateListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev);
	void previewTextTemplateListItemEdited(QListWidgetItem* item);
	void previewTextTemplateNewButtonClicked();
	void previewTextTemplateRemoveButtonClicked();
	void previewTextTemplateSaveButtonClicked();

	void fontOpenButtonClicked();

	void fontShowSystemBoxChanged();
	void fontFamilyListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev);
	void fontStyleListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev);
	void fontSizeListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev);

	void loadedFontDetailsEditAnchorClicked(const QUrl& link);

	void mainSplitterMoved();
	void fontSplitterMoved();

private:
	void updateAvailableFonts(const QString& selectedFamily = QString());
	void updateUsedFont();

	void updatePreviewTextTemplateButtons();

	void updateFontStyleList();
	void updateFontSizeList();

	void savePreviewTextTemplates();

	void updateWindowTitle();

	void updateLoadedFontDetails();

private:
	Ui_MainWindow ui;

	QFontDatabase fdb;
	bool inhibitFontListRebuildOnChange;
	bool unsavedPreviewTextChanges;

	QStringList customFontFamilies;
	QList<LoadedFontFile> loadedFontFiles;
};

#endif /* FONTPREVIEW_MAINWINDOW_H_ */
