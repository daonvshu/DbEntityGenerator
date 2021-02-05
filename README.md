辅助[QtDao](https://github.com/daonvshu/QtDao)使用的代码生成器程序，输入:
```
::生成器 entity配置文件 qtdao库include目录（可省略）
DbEntityGenerator.exe sqlite_entity.xml ../qtdao/src
```

### 字段类型对应关系与默认值配置规则  
注意：entity默认值会影响insert操作，数据库默认值会影响创建/升级数据库操作；autoincrement标记不会生成数据库默认值
#### - sqlite
|字段类型|c++类型|默认值示例|entity默认值|数据库默认值|备注|
|:--:|:--:|:--|:--|:--|:--|
|int|int|-1|-1|-1|
|||null|int()|null|
|long|qint64|0|0|0|
|||null|qint64()|null|
|real|qreal|10.0|10.0|10.0|
|||null|qreal()|null|
|text|QString|string|"string"|'string'|一般字符串|
|||null|QString()|null|null标记|
|||empty|QString()|''|empty标记|
|||"null"|"null"|'null'|使用双引号的字符串|
|||QString("string")|QString("string")|null|使用类名的类构造/静态函数调用|
|blob|QByteArray|abc|"abc"|'abc'|
|||null|QByteArray()|null|
|||empty|QByteArray()|''|
|||"null"|"null"|'null'|
|||QByteArray("abc")|QByteArray("abc")|null|
|variant|QVariant|(any string)|(any string)|null|不做处理的任意字符串|
#### - mysql
|字段类型|c++类型|默认值示例|entity默认值|数据库默认值|备注|
|:--:|:--:|:--|:--|:--|:--|
|tinyint|char|0|0|0|
|||null|char()|null|
|smallint|short|0|0|0|
|||null|short()|null|
|mediumint/int|int|0|0|0|
|||null|int()|null|
|bigint|qint64|0|0|0|
|||null|qint64()|null|
|float/double/decimal|qreal|0.0|0.0|0.0|
|||null|qreal()|null|
|time|QString|120:59:59|"120:59:59"|'120:59:59'|使用时间字符串|
|||QTime::currentTime()<br>.toString("HH:mm:ss")|QTime::currentTime()<br>.toString("HH:mm:ss")|null|使用类名的类构造/静态函数调用|
|||now|QTime::currentTime()<br>.toString("HH:mm:ss")|null|当前时间标志|
|||null|QString()|null|
|date|QDate|2020-01-01|QDate::fromString("2020-01-01")|'2020-01-01'|
|||QDate::currentDate()|QDate::currentData()|null|使用类名的类构造/静态函数调用|
|||now|QDate::currentData()|null|
|||null|QDate()|null|
|datetime/timestamp|QDateTime|2020-01-01 12:59:58.233|QDateTime::fromString<br>("2020-01-01 12:59:58.233")|'2020-01-01 12:59:58.233'|
|||QDateTime::currentDataTime()|QDateTime::currentDataTime()|null|使用类名的类构造/静态函数调用|
|||null|QDateTime()|null|
|||now|QDateTime::currentDateTime()|CURRENT_TIMESTAMP|
|||CURRENT_TIMESTAMP(n)|QDateTime::currentDateTime()|CURRENT_TIMESTAMP(n)|使用数据库本地时间作为默认值|
|||CURRENT_TIMESTAMP(n) ON<br> UPDATE CURRENT_TIMESTAMP(n)|QDateTime::currentDateTime()|CURRENT_TIMESTAMP(n) ON<br> UPDATE CURRENT_TIMESTAMP(n)|默认值和更新都使用数据库本地时间|
|||null ON UPDATE <br>CURRENT_TIMESTAMP(n)|QDateTime()|null ON UPDATE <br>CURRENT_TIMESTAMP(n)|
|char|QChar|a|'a'|'a'|
|||null|QChar()|null|
|varchar|QString|aaa|"aaa"|'aaa'|
|||null|QString()|null|
|||empty|QString()|''|
|||"null"|"null"|'null'|
|||QString("string")|QString("string")|null|
|tinytext/text/mediumtext/longtext|QString|aaa|"aaa"|null|
|||null|QString()|null|
|||empty|QString()|null|
|||"null"|"null"|null|
|||QString("string")|QString("string")|null|
|tinyblob/blob/<br>mediumblob/longblob|QByteArray|aaa|"aaa"|null|
|||null|QByteArray()|null|
|||empty|QByteArray()|null|
|||"null"|"null"|null|
|||QByteArray("string")|QByteArray("string")|null|
#### - sqlserver
|字段类型|c++类型|默认值示例|entity默认值|数据库默认值|备注|
|:--:|:--:|:--|:--|:--|:--|
|tinyint|uchar|0|0|0|
|||null|uchar()|null|
|smallint|short|0|0|0|
|||null|short()|null|
|int|int|0|0|0|
|||null|int()|null|
|bigint|qint64|0|0|0|
|||null|qint64()|null|
|float/double/decimal/<br>numeric/real|qreal|0.0|0.0|0.0|
|||null|qreal()|null|
|time|QTime|12:59:59.6789|QTime::fromString("12:59:59.6789")|'12:59:59.6789'|
|||QTime::currentTime()|QTime::currentTime()|null|使用类名的类构造/静态函数调用|
|||now|QTime::currentTime()|getdate()|当前时间标志|
|||null|QTime()|null|
|date|QDate|2021-01-01|QDate::fromString("2021-01-01")|'2021-01-01'|
|||QDate::currentDate()|QDate::currentDate()|null|
|||now|QDate::currentDate()|getdate()|
|||null|QDate()|null|
|datetime/datetime2/<br>datetimeoffset|QDateTime|2021-01-01 12:59:59.6789|QDateTime::fromString<br>("2021-01-01 12:59:59.6789")|'2021-01-01 12:59:59.6789'|
|||QDateTime::currentDateTime()|QDateTime::currentDateTime()|null|
|||now|QDateTime::currentDateTime()|getdate()|
|||null|QDateTime()|null|
|timestamp|QByteArray|(any string)|QByteArray()|null|
|char/varchar/varchar(max)/<br>nchar/nvarchar/nvarchar(max)/<br>text/ntext|QString|string|"string"|'string'|
|||null|QString()|null|
|||empty|QString()|''|
|||"null"|"null"|'null'|
|||QString("string")|QString("string")|null|
|bit|bool|0|0|0|
|||null|bool()|null|
|binary/varbinary/<br>varbinary(max)|QByteArray|string|"string"|null|
|||null|QByteArray()|null|
|||empty|QByteArray()|null|
|||"null"|"null"|null|
|||QByteArray("string")|QByteArray("string")|null|
|sql_variant|QVariant|(any string)|(any string)|null|
|uniqueidentifier/xml|QByteArray|(any string)|(any string)|null|