using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Mail;
using CommandLine;

namespace lab5
{
    class Program
    {
        private class Options
        {
            [Option('t', "to", Required = true, HelpText = "Receiver.")]
            public string To { get; set; }
            
            [Option('f', "from", Required = true, HelpText = "Sender.")]
            public string From { get; set; }

            [Option('p', "password", Required = true, HelpText = "Password of sender.")]
            public string Password { get; set; }
            
            [Option('k', "keyword", Required = false, Default = "", HelpText = "Keyword.")]
            public string Keyword { get; set; }
        }
        
        static void Main(string[] args)
        {
            Parser.Default.ParseArguments<Options>(args).WithParsed(RunOptions);
        }
        
        static void RunOptions(Options opts)
        {
            Console.WriteLine("Введите отображаемое имя");
            var name = Console.ReadLine();
            
            var from = new MailAddress(opts.From, name);
            var to = new MailAddress(opts.To);
            
            // сообщение
            var m = new MailMessage(from, to);
            
            Console.WriteLine("Введите тему письма");
            m.Subject = Console.ReadLine();
            
            Console.WriteLine("Введите текст письма");
            m.Body = Console.ReadLine();

            if (opts.Keyword != "")
            {
                var files = Directory.GetFiles(".");
                foreach (var file in files)
                {
                    var lines = File.ReadAllLines(file);
                    var findLine = lines.FirstOrDefault(l => l.Contains(opts.Keyword));
                    if (findLine != null)
                    {
                        m.Attachments.Add(new Attachment(file));
                    }
                }
            }

            // smtp-сервер и порт
            SmtpClient smtp = new SmtpClient("smtp.mail.ru", 2525);
            // аутентификационные данные
            smtp.Credentials = new NetworkCredential(opts.From, opts.Password);
            // использование протокола ssl
            smtp.EnableSsl = true;
            
            // отправляем
            smtp.Send(m);
            Console.Write("Отправлено!");
            smtp.Dispose();
        }
    }
}
