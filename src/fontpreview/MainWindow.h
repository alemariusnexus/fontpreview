#ifndef FONTPREVIEW_MAINWINDOW_H_
#define FONTPREVIEW_MAINWINDOW_H_

#include <QMainWindow>
#include <QListWidget>
#include <ui_MainWindow.h>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

	void openFontFile(QString fontFile);
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

	void mainSplitterMoved();

private:
	void updateAvailableFonts(const QString& selectedFamily = QString());
	void updateUsedFont();

	void updatePreviewTextTemplateButtons();

	void updateFontStyleList();
	void updateFontSizeList();

	void savePreviewTextTemplates();

	void updateWindowTitle();

private:
	Ui_MainWindow ui;

	QFontDatabase fdb;
	bool inhibitFontListRebuildOnChange;
	bool unsavedPreviewTextChanges;

	QStringList customFontFamilies;
};

#endif /* FONTPREVIEW_MAINWINDOW_H_ */
