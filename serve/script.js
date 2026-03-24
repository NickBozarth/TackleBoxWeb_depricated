var serverPublicKey = undefined;


// Source - https://stackoverflow.com/a/15724300
// Posted by kirlich, modified by community. See post 'Timeline' for change history
// Retrieved 2026-03-23, License - CC BY-SA 4.0

function getCookie(name) {
    const value = `; ${document.cookie}`;
    const parts = value.split(`; ${name}=`);
    if (parts.length === 2) return parts.pop().split(';').shift();
}


async function importSpkiPublicKey() {
    // extract server-public-key from cookie
    const serverPublicKeyStr = getCookie("server-public-key");

    // extract base64 string
    const b64 = serverPublicKeyStr
        .replaceAll("%20", " ")
        .replaceAll("%2F", "/")
        .replace(/-----BEGIN PUBLIC KEY-----/, "")
        .replace(/-----END PUBLIC KEY-----/, "")
        .replace(/\s/g, "");
    const byteString = window.atob(b64);

    // convert base64 to array
    const bytes = new Uint8Array(byteString.length);
    for (let i = 0; i < byteString.length; i++) {
        bytes[i] = byteString.charCodeAt(i);
    }

    serverPublicKey = await window.crypto.subtle.importKey(
        "spki",
        bytes.buffer,
        {
            name: "RSA-OAEP",
            hash: "SHA-256",
        },
        true,
        ["encrypt"]
    );
}

async function encryptBody(body) {
    // make sure server public key is defined
    if (serverPublicKey === undefined) await importSpkiPublicKey();

    // encode body
    const encoder = new TextEncoder();
    const encodedBody = encoder.encode(body);
    
    // encrypt buffer
    let encryptedBuffer =  await window.crypto.subtle.encrypt(
        { name : "RSA-OAEP" },
        serverPublicKey,
        encodedBody
    );

    const encryptedBytes = new Uint8Array(encryptedBuffer);
    const encryptedBase64 = btoa(String.fromCharCode(...encryptedBytes));
    console.log("HEX SENT:", Array.from(encryptedBytes).map(b => b.toString(16).padStart(2, '0')).join(''));
    return encryptedBase64;
}



async function fetchEncrypted(path, body, method, headers) {
    // make sure unnecessary fields exists
    if (method  === undefined) method  = "POST";
    if (headers === undefined) headers = {};

    // encrypt data
    const encryptedBody = await encryptBody(body); 
    
    // console.log(encryptedBody);
    // for (let byte of encryptedBody) {
    //     console.log(`${byte} `);
    // }

    // fetch data
    return await fetch(
        path,
        {
            method  : "POST",
            headers : headers,
            body    : encryptedBody,
        }
    );
}