Translations
============

The BitRadio Core project has been designed to support multiple localisations. This makes adding new phrases, and completely new languages easily achievable. For managing all application translations, BitRadio Core makes use of the Crowdin online translation management tool.

### Helping to translate (using Crowdin)
Crowdin is setup to monitor the GitHub repo for updates, and when code containing new translations is found, Crowdin will process any changes. It may take several hours after a pull-request has been merged, to appear in the Crowdin web interface.

Multiple language support is critical in assisting BitRadio's global adoption, and growth. One of BitRadio's greatest strengths is cross-border money transfers, any help making that easier is greatly appreciated.

See the [Crowdin BitRadio project](https://crowdin.com/project/bitradio-wallet) to assist in translations.

### Writing code with translations
We use automated scripts to help extract translations in both Qt, and non-Qt source files. It is rarely necessary to manually edit the files in `src/qt/locale/`. The translation source files must adhere to the following format:
`bitradio_xx_YY.ts or bitradio_xx.ts`

`src/qt/locale/bitradio_en.ts` is treated in a special way. It is used as the source for all other translations. Whenever a string in the source code is changed, this file must be updated to reflect those changes. A custom script is used to extract strings from the non-Qt parts. This script makes use of `gettext`, so make sure that utility is installed (ie, `apt-get install gettext` on Ubuntu/Debian). Once this has been updated, `lupdate` (included in the Qt SDK) is used to update `bitradio_en.ts`.

To automatically regenerate the `bitradio_en.ts` file, run the following commands:
```sh
cd src/
make translate
```

`contrib/bitradio-qt.pro` takes care of generating `.qm` (binary compiled) files from `.ts` (source files) files. It’s mostly automated, and you shouldn’t need to worry about it.

**Example Qt translation**
```cpp
QToolBar *toolbar = addToolBar(tr("Tabs toolbar"));
```

### Creating a pull-request
For general PRs, you shouldn’t include any updates to the translation source files. They will be updated periodically, primarily around pre-releases, allowing time for any new phrases to be translated before public releases. This is also important in avoiding translation related merge conflicts.

When an updated source file is merged into the GitHub repo, Crowdin will automatically detect it (although it can take several hours). Once processed, the new strings will show up as "Remaining" in the Crowdin web interface and are ready for translators.

To create the pull-request, use the following commands:
```
git add src/qt/bitradiostrings.cpp src/qt/locale/bitradio_en.ts
git commit
```

### Handling Plurals (in source files)
When new plurals are added to the source file, it's important to do the following steps:

1. Open `bitradio_en.ts` in Qt Linguist (included in the Qt SDK)
2. Search for `%n`, which will take you to the parts in the translation that use plurals
3. Look for empty `English Translation (Singular)` and `English Translation (Plural)` fields
4. Add the appropriate strings for the singular and plural form of the base string
5. Mark the item as done (via the green arrow symbol in the toolbar)
6. Repeat from step 2, until all singular and plural forms are in the source file
7. Save the source file

### Translating a new language
To create a new language template, you will need to edit the languages manifest file `src/qt/bitradio_locale.qrc` and add a new entry. Below is an example of the English language entry.

```xml
<qresource prefix="/translations">
    <file alias="en">locale/bitradio_en.qm</file>
    ...
</qresource>
```

**Note:** that the language translation file **must end in `.qm`** (the compiled extension), and not `.ts`.