declare namespace t = "http://schemas.microsoft.com/exchange/services/2006/types";
declare namespace m = "http://schemas.microsoft.com/exchange/services/2006/messages";
declare namespace soap = "http://schemas.xmlsoap.org/soap/envelope/";
if (/soap:Envelope/soap:Body/m:GetFolder and
    //m:GetFolder/m:FolderShape/t:BaseShape = <t:BaseShape>IdOnly</t:BaseShape> and
    not(//t:AdditionalProperties) and
    count(//m:GetFolder/m:FolderIds/t:DistinguishedFolderId) = 0 and
    count(//m:GetFolder/m:FolderIds/t:FolderId) = %1 and
    %2
) then (
    <soap:Envelope><soap:Header>
    <t:ServerVersionInfo MajorVersion="15" MinorVersion="01" MajorBuildNumber="225" MinorBuildNumber="042" />
    </soap:Header><soap:Body>
    <m:GetFolderResponse>
    <m:ResponseMessages>
    %3
    </m:ResponseMessages>
    </m:GetFolderResponse></soap:Body></soap:Envelope>
) else ()