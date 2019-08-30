/**
 * \file spellchecker.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2014-2019 The InyokaEdit developers
 *
 * This file is part of InyokaEdit.
 *
 * InyokaEdit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InyokaEdit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InyokaEdit.  If not, see <https://www.gnu.org/licenses/>.
 *
 * \section DESCRIPTION
 * Class definition for spell checker using hunspell.
 *
 * \section SOURCE
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright (c) 2011, Volker Götz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Original code form: https://wiki.qt.io/Spell-Checking-with-Hunspell
 */

#ifndef PLUGINS_SPELLCHECKER_SPELLCHECKER_H_
#define PLUGINS_SPELLCHECKER_SPELLCHECKER_H_

#include <QObject>
#include <QTranslator>
#include <QtPlugin>
#include <QSettings>
#include <QString>
#include <QTextCursor>

#include "../../application/texteditor.h"
#include "../../application/ieditorplugin.h"
#include "./spellcheckdialog.h"

#if defined _WIN32
#include "../windows_files/hunspell-mingw/include/hunspell.hxx"
#else
#include <hunspell/hunspell.hxx>
#endif

/**
 * \class SpellChecker
 * \brief Spell checker using hunspell.
 */
class SpellChecker : public QObject, IEditorPlugin {
  Q_OBJECT
  Q_INTERFACES(IEditorPlugin)
  Q_PLUGIN_METADATA(IID "InyokaEdit.spellchecker")

 public:
    ~SpellChecker();

    void initPlugin(QWidget *pParent, TextEditor *pEditor,
                    const QDir &userDataDir, const QString &sSharePath);
    QString getPluginName() const;
    QString getPluginVersion() const;
    void installTranslator(const QString &sLang);
    QString getCaption() const;
    QIcon getIcon() const;
    bool includeMenu() const;
    bool includeToolbar() const;
    bool hasSettings() const;
    void setCurrentEditor(TextEditor *pEditor);
    void setEditorlist(const QList<TextEditor *> &listEditors);

 public slots:
    void callPlugin();
    void executePlugin();
    void showSettings();
    void showAbout();

 private:
    friend class SpellCheckDialog;

    void setDictPath();
    bool initDictionaries();
    void loadAdditionalDict(const QString &sFilename);

    bool spell(const QString &sWord);
    QStringList suggest(const QString &sWord);
    void ignoreWord(const QString &sWord);
    void addToUserWordlist(const QString &sWord);
    void putWord(const QString &sWord);
    void replaceAll(const int nPos, const QString &sOld, const QString &sNew);

    Hunspell *m_pHunspell;
    TextEditor *m_pEditor;
    QAction *m_pExecuteAct;
    SpellCheckDialog *m_pCheckDialog;
    QSettings *m_pSettings;
    QTextCursor m_oldCursor;
    QString m_sDictPath;
    QStringList m_sListDicts;
    QString m_sDictLang;
    QString m_sUserDict;
    QDir m_UserDataDir;
    QTranslator m_translator;
    QString m_sSharePath;
    QString m_sCommunity;
    QString m_sEncoding;
    QTextCodec *m_pCodec;
};

#endif  // PLUGINS_SPELLCHECKER_SPELLCHECKER_H_