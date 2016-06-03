
#ifndef DIALOG_REPLACE_H_
#define DIALOG_REPLACE_H_

#include <QDialog>
#include <QPointer>
#include "SearchType.h"
#include "SearchDirection.h"
#include <ctime>

#ifdef REPLACE_SCOPE
#include "ui_DialogReplaceScope.h"
#include "ReplaceScope.h"
#else
#include "ui_DialogReplace.h"
#endif



class Document;
class DialogMultiReplace;

class DialogReplace : public QDialog {
	Q_OBJECT
public:
	DialogReplace(Document *window, QWidget *parent = 0, Qt::WindowFlags f = 0);
	virtual ~DialogReplace();
	
protected:
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void showEvent(QShowEvent *event) override;
	
public:
	void setTextField(Document *window, time_t time);
	void initToggleButtons(SearchType searchType);
	void fUpdateActionButtons();
#ifdef REPLACE_SCOPE
	void rSetActionButtons(bool replaceBtn, bool replaceFindBtn, bool replaceAndFindBtn, bool replaceAllBtn);
#else
	void rSetActionButtons(bool replaceBtn, bool replaceFindBtn, bool replaceAndFindBtn, bool replaceInWinBtn, bool replaceInSelBtn, bool replaceAllBtn);
#endif
	void UpdateReplaceActionButtons();
	int getReplaceDlogInfo(SearchDirection *direction, char *searchString, char *replaceString, SearchType *searchType);
	void collectWritableWindows();

public:
	bool keepDialog() const;

private Q_SLOTS:
	void on_checkBackward_toggled(bool checked);
	void on_checkRegex_toggled(bool checked);
	void on_checkCase_toggled(bool checked);
	void on_checkKeep_toggled(bool checked);
	void on_textFind_textChanged(const QString &text);
	void on_buttonFind_clicked();
	void on_buttonReplace_clicked();
	void on_buttonReplaceFind_clicked();
#ifdef REPLACE_SCOPE
	void on_buttonAll_clicked();
	void on_radioWindow_toggled(bool checked);
	void on_radioSelection_toggled(bool checked);
	void on_radioMulti_toggled(bool checked);
#else
	void on_buttonWindow_clicked();
	void on_buttonSelection_clicked();
	void on_buttonMulti_clicked();
#endif
	
public:
	Document *window_;
#ifdef REPLACE_SCOPE
	Ui::DialogReplaceScope ui;
	ReplaceScope replaceScope_;
#else
	Ui::DialogReplace ui;
#endif
	bool lastRegexCase_;        /* idem, for regex mode in find dialog */
	bool lastLiteralCase_;      /* idem, for literal mode */
	QPointer<DialogMultiReplace> dialogMultiReplace_;	
};


#endif
