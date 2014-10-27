#pragma once

//yes 1, no 0
extern i32 showYesNoBox(const nString& message);
extern i32 showYesNoCancelBox(const nString& message);
extern void showMessage(const nString& message);

extern nString getFullPath(const cString initialDir);
extern void pError(const cString message);
extern void pError(const nString& message);

extern bool checkGlError(const nString& errorLocation);