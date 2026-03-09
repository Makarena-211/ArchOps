workspace "Lab1" {
    !identifiers hierarchical
    model {
        user = person "Пользователь"
        patient = person "Пациент"
        doctor = person "Врач"
        admin = person "Админ"

        MedicineSystem = softwareSystem "Медицинская Система"{
            webApp = container "WebApp" "Web приложение" "React" "Для врачей и пациентов (frontend)"
            api = container "RestApi" "Api server" "FastApi" "Бизнес логика (Backend)"
            auth = container "Auth" "Сервис Аутентификации" "OAuth2/JWT" "Управление логином, токенами, ролями"
            db = container "DB" "База Данных" "PostgreSQL" "Хранение пользователей, пациентов, записей"
            files = container "FilesStorage" "Хранилище для файлов" "S3" "Хранение вложений (сканы, изображения, исследования)"
            cache = container "Cache" "Кэш для авторизации и частых запросов" "Redis"
            adminApp = container "AdminConsole" "Приложение для админа" "Fastapi" "Приложение для админа"
            worker = container "BackgroundWorker" "Сущность для обработки задач по оплате или уведомлений" "Fastapi" "Отправка SMS, Email, Проведение оплаты"
            
        }

        PaymentSystem = softwareSystem "Система оплаты"{
            tags "External"
        }

        EmailService = softwareSystem "Почтовая рассылка"{
            tags "External"
        }

        SMSService = softwareSystem "SMS рассылка"{
            tags "External"
        }

        user -> MedicineSystem "Регистрируется в системе"
        patient -> MedicineSystem "Просматривает записи\Пользуется своей медкартой"
        doctor -> MedicineSystem "Просматривает медкарты пациентов"
        admin -> MedicineSystem "Имеет мастер права над системой"

        MedicineSystem -> PaymentSystem "Проводит оплату"
        MedicineSystem -> EmailService "Уведомляет о приеме/о записи через email рассылку"
        MedicineSystem -> SMSService "Уведомляет о приеме/о записи через sms рассылку"

        user -> MedicineSystem.webApp "Использует (просмотр)"
        patient -> MedicineSystem.webApp "Использует (личный кабинет)" 
        doctor -> MedicineSystem.webApp "Использует (рабочий интерфейс)"
        MedicineSystem.webApp -> MedicineSystem.api "REST/HTTPS"
        MedicineSystem.api -> MedicineSystem.auth "Проверка токенов / логин"
        MedicineSystem.api -> MedicineSystem.db "Чтение/запись данных (SQL)(PG Protocol)"
        MedicineSystem.api -> MedicineSystem.files "Загрузка/получение файлов (REST/HTTPS)"
        MedicineSystem.api -> MedicineSystem.cache "Кэширование запросов (RESP)"
        MedicineSystem.api -> MedicineSystem.worker "Передача операций на выполнение для интеграций"
        MedicineSystem.worker -> EmailService "Отправляет email (API)(REST/HTTPS)"
        MedicineSystem.worker -> SMSService "Отправляет SMS (API)(REST/HTTPS)"
        MedicineSystem.api -> PaymentSystem "Синхронный вызов оплаты (REST) или редирект на стороннюю страницу (REST/HTTPS)"
        MedicineSystem.adminApp -> MedicineSystem.api "(управление)(REST/HTTPS)"
    }


    views {
        systemContext MedicineSystem "SystemContext" "Контекст системы" {
            include *
            autoLayout
        }


        container MedicineSystem "Containers" "Контейнеры медицинской системы" {
            include *
            autoLayout
        }

        dynamic MedicineSystem "CreateMedicalRecordSequence" "Диаграмма последовательности: врач создаёт запись, пациент получает уведомление" {
            doctor -> MedicineSystem.webApp "Открывает интерфейс врача и выбирает пациента"
            MedicineSystem.webApp -> MedicineSystem.api "POST /api/records {patientId, diagnosis, notes, attachments}"
            MedicineSystem.api -> MedicineSystem.auth "Validate JWT and check role 'doctor'"
            MedicineSystem.auth -> MedicineSystem.api "200 OK (token valid)"
            MedicineSystem.api -> MedicineSystem.db "INSERT INTO records (patient_id, data...)"
            MedicineSystem.db -> MedicineSystem.api "recordId"
            MedicineSystem.api -> MedicineSystem.files "Загрузить вложения (при наличии) -> получить url(ы)"
            MedicineSystem.api -> MedicineSystem.webApp "201 Created (recordId)"
            MedicineSystem.api -> MedicineSystem.worker "Nees to Send email to patient (record created)"
            MedicineSystem.worker -> EmailService "Send email to patient (record created)"
            MedicineSystem.api -> MedicineSystem.worker "Nees to Send SMS to patient (optional))"
            MedicineSystem.worker -> SMSService "Send SMS to patient (optional)"
            MedicineSystem.api -> MedicineSystem.db "Update notifications table/status"
            autoLayout
        }

        styles {
            element "Element" {
                color #0773af
                stroke #0773af
                strokeWidth 7
                shape roundedbox
            }
            element "Person" {
                shape person
            }
            element "Boundary" {
                strokeWidth 5
            }
            relationship "Relationship" {
                thickness 4
            }
        }
    }

    
}