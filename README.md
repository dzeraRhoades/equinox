# Equinox Framework
Eclipse Equinox Framework - это реализация OSGI R4 для поддержки динамической модульной системы Java приложений.
## Сборка
Пререквизиты:
- Maven
- Tycho
- Visual Studio для Windows
- gcc и make для Linux
- clang и make для MacOS
- JDK с JNI библиотекой (версия не меньше 17)

Для сборки фреймворка без нативной части:

```
mvn clean package -Pbuild-individul-bundles
```
Для сборки с нативным кодом, необходимо указать целевую ОС и архитектуру:

```
mvn clean package -Pbuild-individul-bundles -Dnative=${os.ws.arch}

```
Для windows - win32.win32.x86_64, для linux - gtk.linux.x86_64, для macos - cocoa.macosx.x86_64. На данный момент macos под arm не поддерживается.
