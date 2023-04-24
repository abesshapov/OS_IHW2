# OS_IHW2
Индивидуальное домашнее задание 2 по дисциплине "Операционные системы"
### Выполнил студент группы БПИ219 Бесшапов Алексей Павлович, Вариант 27. Условие:

#### Преподаватель проводит экзамен у группы студентов. Каждый студент получает свой билет, сообщает его номер и готовит письменный ответ. Подготовив ответ, он передает его преподавателю. Преподаватель просматривает ответ и сообщает студенту оценку. Студент, дождавшись результата, уходит с экзамена. Требуется создать приложение, моделирующее действия преподавателя и студентов, каждый из которых представлен отдельным процессом.

### Принцип решения

Смоделируем ситуацию. Экзамен по математике, в пуле билетов 6 билетов. Соответственно, пока билеты есть, ученики заходят и берут билеты, готовясь к ответу. Сдав работу, ученик узнает результат и сдает билет, после чего заходит другой ученик, беря билет. Отразим данную ситуацию на процессы. Учитель становится главным процессом, работает, пока не сдадут все k учеников (передается через консоль). Он проверяет, остались ли билеты и передает эту информацию ученикам - дочерним процессам. Заходя в кабинет (начиная работу), процесс блокирует семафор и берет билет, после чего освобождает семафор и готовится. Подготовившись, он блокирует семафор, имитируя взаимодействие с учителем. Закончив сдачу, он освобождает семафор и отключается. 

### 4 балла

Используются именованные семафоры Posix + разделяемая память в формате Posix. На вход должен быть передан аргумент k - количество учеников - через консоль.

### 5 баллов

Используются неименованные семафоры Posix + разделяемая память в формате Posix. На вход должен быть передан аргумент k - количество учеников - через консоль.

### 6 баллов

Используются семафоры UNIX SYSTEM V + разделяемая память в формате UNIX SYSTEM V. На вход должен быть передан аргумент k - количество учеников - через консоль.

### 7 баллов

Используются именованные семафоры Posix + разделяемая память в формате Posix. Процессы учеников и учителя запускаются отдельными программами. На вход должен быть передан аргумент k - номер ученика - через консоль (при запуске ./student). Для учителя также задано максимальное количество принимающихся учеников - 100.

### 8 баллов

Используются семафоры UNIX SYSTEM V + разделяемая память в формате UNIX SYSTEM V. Процессы учеников и учителя запускаются отдельными программами. На вход должен быть передан аргумент k - номер ученика - через консоль (при запуске ./student). Для учителя также задано максимальное количество принимающихся учеников - 100.

## Итого

Демонстрационные варианты отработанных программ находятся в папке TestsShowcasingScreenshots. Задание выполнено на оценку 8.
