workspace "Name" "Description" {
    
    model {
        user = person "Пользователь"
        patient = person "Пациент"
        doctor = person "Врач"
        admin = person "Админ"

        MedicineSystem = softwareSystem "Медицинская Система"{
            container1 = container "Имя контейнера" "Описание" "Технология"
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
    }

    views {
        systemContext MedicineSystem "SystemContext" "Контекст системы" {
            include *
            autoLayout 
        }

    }
}