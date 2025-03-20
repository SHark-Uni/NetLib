using System;
using System.Collections.Generic;
using System.IO;

class Program
{
    static void Main(string[] args)
    {
        string currentPath = Directory.GetCurrentDirectory();
        int default_message_number = 0;
        // protocol.txt 파일의 전체 경로 생성
        string filePath = Path.Combine(currentPath, "protocol_.txt");
        List<RES_MESSAGE> ResMessages = new List<RES_MESSAGE>();
        List<REQ_MESSAGE> ReqMessages = new List<REQ_MESSAGE>();

        List<string> typedefs = new List<string>();

        string[] lines = File.ReadAllLines(filePath);

        RES_MESSAGE current_RES_MESSAGE = null;
        REQ_MESSAGE current_REQ_MESSAGE = null;

        for (int i = 0; i < lines.Length; i++)
        {
            string line = lines[i].Trim();
            if (line.StartsWith("[PROTOCOL_END]"))
            {
                break;
            }
            if (line.StartsWith("@typedef"))
            {
                string typedefStatement = line.Replace("@typedef", "typedef").Trim();
                typedefs.Add(typedefStatement);
                continue;
            }

            if (line.StartsWith("@RES"))
            {
                current_RES_MESSAGE = new RES_MESSAGE();
                bool insideBraces = false;
                //다음 줄부터 읽기. 하나의 메시지를 해석하면 끝낸다.
                for (int j = i + 1; j < lines.Length; j++)
                {
                    string nextLine = lines[j].Trim();
                    if (nextLine.StartsWith("}"))
                    {
                        ResMessages.Add(current_RES_MESSAGE);
                        insideBraces = false;
                        break;
                    }
                    ProcessLine_RES(nextLine, ref current_RES_MESSAGE, ref insideBraces, ResMessages, default_message_number);
                }
            }
            else if (line.StartsWith("@REQ"))
            {
                current_REQ_MESSAGE = new REQ_MESSAGE();
                bool insideBraces = false;
                //다음 줄부터 읽기. 하나의 메시지를 해석하면 끝낸다.
                for (int j = i + 1; j < lines.Length; j++)
                {
                    string nextLine = lines[j].Trim();
                    if (nextLine.StartsWith("}"))
                    {
                        ReqMessages.Add(current_REQ_MESSAGE);
                        insideBraces = false;
                        break;
                    }
                    ProcessLine_REQ(nextLine, ref current_REQ_MESSAGE, ref insideBraces, ReqMessages, default_message_number);
                }

            }
            ++default_message_number;
        }

        string serverOutputPath = "server_SendMessageProc.cpp";
        string serverOutputHeaderPath = "server_SendMessageProc.h";

        string serverMessageProcPath = "server_RecvMessageProc.cpp";
        string serverMessageProcHeaderPath = "server_RecvMessageProc.h";

        string serverOutputRecvPath = "server_OnRecvProc.cpp";


        string clientOutputPath = "client_SendMessageProc.cpp";
        string clientOutputHeaderPath = "client_SendMessageProc.h";

        string clientMessageProcPath = "client_RecvMessageProc.cpp";
        string clientMessageProcHeaderPath = "client_RecvMessageProc.h";

        string clientOutputRecvPath = "Client_OnRecvProc.cpp";

        string MessageFormatPath = "MessageFormat.h";

        /*Server Send File*/
        GenerateServerCppFile(ResMessages, serverOutputPath);
        GenerateServerHeaderFile(ResMessages, serverOutputHeaderPath, typedefs);

        /*Server Recv File*/
        GenerateServerRecvCppFile(ReqMessages, serverMessageProcPath);
        GenerateServerRecvHeaderFile(ReqMessages, serverMessageProcHeaderPath, typedefs);
        GenerateServerOnRecvProc(ReqMessages, serverOutputRecvPath, typedefs);
        /*Client Send File*/
        GenerateClientSendCppFile(ReqMessages, clientOutputPath);
        GenerateClientSendHeaderFile(ReqMessages, clientOutputHeaderPath, typedefs);

        /*Client Recv File*/
        GenerateClientCppFile(ResMessages, clientMessageProcPath);
        GenerateClientHeaderFile(ResMessages, clientMessageProcHeaderPath, typedefs);
        GenerateClientOnRecvProc(ResMessages, clientOutputRecvPath, typedefs);

        GenerateMessageFormat(ResMessages, ReqMessages, MessageFormatPath);

        foreach (var resMessage in ResMessages)
        {
            resMessage.Display();
            Console.WriteLine(); // 메시지 간 간격 추가
        }
        foreach (var reqMessage in ReqMessages)
        {
            reqMessage.Display();
            Console.WriteLine(); // 메시지 간 간격 추가
        }

        Console.ReadLine();
    }
    static void ProcessLine_RES(string nextLine, ref RES_MESSAGE current_RES_MESSAGE, ref bool insideBraces, List<RES_MESSAGE> ResMessages, int default_message_number)
    {
        if (nextLine.StartsWith("@Uni"))
        {
            current_RES_MESSAGE.Type = "@Uni";
            return;
        }
        else if (nextLine.StartsWith("@Broad"))
        {
            current_RES_MESSAGE.Type = "@Broad";
            return;
        }
        else if (nextLine.StartsWith("@BroadExceptMe"))
        {
            current_RES_MESSAGE.Type = "@Broad";
            return;
        }

        if (current_RES_MESSAGE.Name == null)
        {
            string[] parts = null;
            if (nextLine.Contains(":"))
            {
                parts = nextLine.Split(':');
                current_RES_MESSAGE.Name = parts[0].Trim();
                int.TryParse(parts[1].Trim(), out int messageNumber);
                current_RES_MESSAGE.MessageNumber = messageNumber;
            }
            else
            {
                current_RES_MESSAGE.Name = nextLine;
                current_RES_MESSAGE.MessageNumber = default_message_number;
            }
            return;
        }

        if (nextLine.StartsWith("{"))
        {
            insideBraces = true;
            return;
        }

        if (insideBraces)
        {
            string[] parts = nextLine.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            string type = parts[0].Trim();

            switch (type)
            {
                case "char":
                    current_RES_MESSAGE.MessageLen += 1;
                    break;
                case "bool":
                    current_RES_MESSAGE.MessageLen += 1;
                    break;
                case "short":
                    current_RES_MESSAGE.MessageLen += 2;
                    break;
                case "int":
                    current_RES_MESSAGE.MessageLen += 4;
                    break;
                case "float":
                    current_RES_MESSAGE.MessageLen += 4;
                    break;
                case "long":
                    current_RES_MESSAGE.MessageLen += 4;
                    break;
                case "long long":
                    current_RES_MESSAGE.MessageLen += 8;
                    break;
                case "double":
                    current_RES_MESSAGE.MessageLen += 8;
                    break;
            }

            string name = parts[1].Trim().TrimEnd(','); // 변수 이름
            current_RES_MESSAGE.Details.Add($"{type} {name}"); // 배열에 "타입 변수" 형식으로 저장
            return;
        }
    }
    static void ProcessLine_REQ(string nextLine, ref REQ_MESSAGE current_REQ_MESSAGE, ref bool insideBraces, List<REQ_MESSAGE> ReqMessages, int default_message_number)
    {
        if (nextLine.StartsWith("}"))
        {
            ReqMessages.Add(current_REQ_MESSAGE);
            insideBraces = false;
            return;
        }

        if (current_REQ_MESSAGE.Name == null)
        {
            string[] parts = null;
            if (nextLine.Contains(":"))
            {
                parts = nextLine.Split(':');
                current_REQ_MESSAGE.Name = parts[0].Trim();
                int.TryParse(parts[1].Trim(), out int messageNumber);
                current_REQ_MESSAGE.MessageNumber = messageNumber;
            }
            else
            {
                current_REQ_MESSAGE.Name = nextLine;
                current_REQ_MESSAGE.MessageNumber = default_message_number;
            }
            return;
        }

        if (nextLine.StartsWith("{"))
        {
            insideBraces = true;
            return;
        }

        if (insideBraces)
        {
            string[] parts = nextLine.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            string type = parts[0].Trim();

            string name = parts[1].Trim().TrimEnd(','); // 변수 이름
            current_REQ_MESSAGE.Details.Add($"{type} {name}"); // 배열에 "타입 변수" 형식으로 저장
            return;
        }
    }
    
    static void GenerateServerCppFile(List<RES_MESSAGE> resMessages, string filePath)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            writer.WriteLine("// 서버 송신부 파일");
            writer.WriteLine("#include \"server_SendMessageProc.h\"");
            writer.WriteLine();

            foreach (var message in resMessages)
            {
                writer.WriteLine($"void RPC_{message.Name}(SerializeBuffer* message, {string.Join(", ", message.Details)}, SESSION_KEY key)");
                writer.WriteLine("{");
                writer.WriteLine("     //Message Header");

                writer.WriteLine($"    *message << 0x89");
                writer.WriteLine($"    *message << {message.MessageLen}");
                writer.WriteLine($"    *message << {message.MessageNumber} //메시지 번호");

                // 각 필드를 SerializeBuffer에 추가
                foreach (var detail in message.Details)
                {
                    string variableName = detail.Split(' ')[1]; // 변수 이름 추출
                    writer.WriteLine($"    *message << {variableName};");
                }

                if (message.Type.Equals("@Uni"))
                {
                    writer.WriteLine("    SendUniCast(key, message, message->getUsedSize());");
                }
                else if (message.Type.Equals("@Broad"))
                {
                    writer.WriteLine("    SendBroadCast(message, message->getUsedSize());");
                }
                else if (message.Type.Equals("BroadExceptMe"))
                {
                    writer.WriteLine("    SendBroadCast(key, message, message->getUsedSize());");
                }

                writer.WriteLine("}");
                writer.WriteLine();
            }
        }
    }
    static void GenerateServerHeaderFile(List<RES_MESSAGE> resMessages, string filePath, List<string> typedef)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            writer.WriteLine("typedef int SESSION_KEY;");
            foreach (string item in typedef)
            {
                writer.WriteLine($"{item};");
            }
            foreach (var message in resMessages)
            {
                //RES를 돌면서,
                writer.WriteLine($"void RPC_{message.Name}(SerializeBuffer* message, {string.Join(", ", message.Details)}, SESSION_KEY key);");
            }
        }
    }
    static void GenerateServerRecvCppFile(List<REQ_MESSAGE> reqMessages, string filePath)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            writer.WriteLine("// 서버 수신부 파일");
            writer.WriteLine("#include \"server_RecvMessageProc.h\"");
            writer.WriteLine();

            foreach (var message in reqMessages)
            {
                writer.WriteLine($"void RPC_{message.Name}(SerializeBuffer* message, SESSION_KEY key)");
                writer.WriteLine("{");

                // 각 필드를 DeserializeBuffer에서 읽기
                foreach (var detail in message.Details)
                {
                    string[] parts = detail.Split(' ');
                    string type = parts[0]; // 변수 타입
                    string variableName = parts[1]; // 변수 이름
                    writer.WriteLine($"    {type} {variableName};");
                    writer.WriteLine($"    *message >> {variableName};");
                }
                writer.WriteLine("}");
                writer.WriteLine();
            }
        }
    }
    static void GenerateServerRecvHeaderFile(List<REQ_MESSAGE> reqMessages, string filePath, List<string> typedef)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            writer.WriteLine("typedef int SESSION_KEY");
            foreach (string item in typedef)
            {
                writer.WriteLine($"{item};");
            }
            foreach (var message in reqMessages)
            {
                writer.WriteLine($"void RPC_{message.Name}(SerializeBuffer* message, SESSION_KEY key);");
            }
        }
    }
    static void GenerateServerOnRecvProc(List<REQ_MESSAGE> reqMessages, string filePath, List<string> typedef)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            writer.WriteLine("// 서버 수신부 MessageProc");
            writer.WriteLine("#include \"MessageFormat.h\"");
            writer.WriteLine("typedef int SESSION_KEY");
            foreach (string item in typedef)
            {
                writer.WriteLine($"{item};");
            }
            writer.WriteLine();

            writer.WriteLine($"void RPC_MessageProc(SerializeBuffer* message, MESSAGE_TYPE msgtype, SESSION_KEY key)");
            writer.WriteLine("{");
            writer.WriteLine("    switch(msgtype)");
            writer.WriteLine("    {");
            foreach (var procedure in reqMessages)
            {
                writer.WriteLine($"    case df_REQ_{procedure.Name} :");
                writer.WriteLine($"        RPC_{procedure.Name}(SerializeBuffer * message, SESSION_KEY key);");
                writer.WriteLine($"        break;");
            }
            writer.WriteLine("        default:");
            writer.WriteLine("        OnDestroyProc(key);");
            writer.WriteLine("        break;");
            writer.WriteLine("    }");
            writer.WriteLine("}");
            writer.WriteLine();

        }
    }

    static void GenerateClientSendCppFile(List<REQ_MESSAGE> reqMessages, string filePath)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            writer.WriteLine("// 클라 송신부 파일");
            writer.WriteLine("#include \"client_SendMessageProc.h\"");
            writer.WriteLine();

            foreach (var message in reqMessages)
            {
                writer.WriteLine($"void RPC_{message.Name}(SerializeBuffer* message, {string.Join(", ", message.Details)})");
                writer.WriteLine("{");
                writer.WriteLine("     //Message Header");

                writer.WriteLine($"    *message << 0x89");
                writer.WriteLine($"    *message << {message.MessageLen}");
                writer.WriteLine($"    *message << {message.MessageNumber} //메시지 번호");

                // 각 필드를 SerializeBuffer에 추가
                foreach (var detail in message.Details)
                {
                    string variableName = detail.Split(' ')[1]; // 변수 이름 추출
                    writer.WriteLine($"    *message << {variableName};");
                }
                writer.WriteLine("    SendUniCast(g_Socket, message, message->getUsedSize());");
                writer.WriteLine("}");
                writer.WriteLine();
            }
        }
    }
    static void GenerateClientSendHeaderFile(List<REQ_MESSAGE> reqMessages, string filePath, List<string> typedef)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            foreach (string item in typedef)
            {
                writer.WriteLine($"{item};");
            }
            foreach (var message in reqMessages)
            {
                //RES를 돌면서,
                writer.WriteLine($"void RPC_{message.Name}(SerializeBuffer* message, {string.Join(", ", message.Details)});");
            }
        }
    }

    static void GenerateClientCppFile(List<RES_MESSAGE> resMessages, string filePath)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            writer.WriteLine("// 클라이언트 수신부 파일");
            writer.WriteLine("#include \"client_RecvMessageProc.h\"");
            writer.WriteLine();

            foreach (var message in resMessages)
            {
                writer.WriteLine($"void RPC_{message.Name}(SerializeBuffer* message)");
                writer.WriteLine("{");

                // 각 필드를 DeserializeBuffer에서 읽기
                foreach (var detail in message.Details)
                {
                    string[] parts = detail.Split(' ');
                    string type = parts[0]; // 변수 타입
                    string variableName = parts[1]; // 변수 이름
                    writer.WriteLine($"    {type} {variableName};");
                    writer.WriteLine($"    *message >> {variableName};");
                }
                writer.WriteLine("}");
                writer.WriteLine();
            }
        }
    }
    static void GenerateClientHeaderFile(List<RES_MESSAGE> resMessages, string filePath, List<string> typedef)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            foreach (string item in typedef)
            {
                writer.WriteLine($"{item};");
            }
            foreach (var message in resMessages)
            {
                writer.WriteLine($"void RPC_{message.Name}(SerializeBuffer* message);");
            }
        }
    }
    static void GenerateClientOnRecvProc(List<RES_MESSAGE> resMessages, string filePath, List<string> typedef)
    {
        using (StreamWriter writer = new StreamWriter(filePath))
        {
            writer.WriteLine("// 클라이언트 수신부 MessageProc");
            writer.WriteLine("#include \"MessageFormat.h\"");
            foreach (string item in typedef)
            {
                writer.WriteLine($"{item};");
            }
            writer.WriteLine();

            writer.WriteLine($"void RPC_MessageProc(SerializeBuffer* message, MESSAGE_TYPE msgtype)");
            writer.WriteLine("{");
            writer.WriteLine("    switch(msgtype)");
            writer.WriteLine("    {");
            foreach (var procedure in resMessages)
            {
                writer.WriteLine($"    case df_RES_{procedure.Name} :");
                writer.WriteLine($"        RPC_{procedure.Name}(SerializeBuffer * message);");
                writer.WriteLine($"        break;");
            }
            writer.WriteLine("        default:");
            writer.WriteLine("        break;");
            writer.WriteLine("    }");
            writer.WriteLine("}");
            writer.WriteLine();

        }
    }
    static void GenerateMessageFormat(List<RES_MESSAGE> resMessages, List<REQ_MESSAGE> reqMessages, string filePath)
    {
        Dictionary<int, string> messageFormat = new Dictionary<int, string>();

        foreach (var reqMessage in reqMessages)
        {
            if (!messageFormat.ContainsKey(reqMessage.MessageNumber))
            {
                messageFormat[reqMessage.MessageNumber] = $"REQ_{reqMessage.Name}";
            }
        }

        //RES를 돌면서, #define df_RES_함수이름 (번호)
        foreach (var resMessage in resMessages)
        {
            if (!messageFormat.ContainsKey(resMessage.MessageNumber))
            {
                messageFormat[resMessage.MessageNumber] = $"RES_{resMessage.Name}";
            }
        }

        using (StreamWriter writer = new StreamWriter(filePath))
        {
            writer.WriteLine("#pragma once");
            foreach (var kv in messageFormat)
            {
                writer.WriteLine($"#define df_{kv.Value} ({kv.Key})");
            }
        }
    }
}
public class RES_MESSAGE
{
    public string Name { get; set; } // "CreateMyCharacter"와 같은 메시지 이름
    public string Type { get; set; } // @Uni -> 1, @Broad -> 2, @BroadExceptMe -> 3
    public List<string> Details { get; set; } // 중괄호 내 변수와 타입 정보
    public int MessageNumber;
    public int MessageLen;
    public RES_MESSAGE()
    {
        Details = new List<string>();
        MessageLen = 0;
    }
    public void Display()
    {
        Console.WriteLine($"Name: {Name}");
        Console.WriteLine($"Type: {Type}");
        Console.WriteLine($"Message Number : {MessageNumber}");
        Console.WriteLine("Details:");
        foreach (string detail in Details)
        {
            Console.WriteLine($"  {detail}");
        }
    }
}

public class REQ_MESSAGE
{
    public string Name { get; set; } // "CreateMyCharacter"와 같은 메시지 이름
    public string Type { get; set; } // @Uni -> 1, @Broad -> 2, @BroadExceptMe -> 
    public List<string> Details { get; set; } // 중괄호 내 변수와 타입 정보
    public int MessageNumber;
    public int MessageLen;
    public REQ_MESSAGE()
    {
        Details = new List<string>();
    }
    public void Display()
    {
        Console.WriteLine($"Name: {Name}");
        Console.WriteLine($"Type: {Type}");
        Console.WriteLine($"Message Number : {MessageNumber}");
        Console.WriteLine("Details:");
        foreach (string detail in Details)
        {
            Console.WriteLine($"  {detail}");
        }
    }
}


