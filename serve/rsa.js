var serverPublicKey = undefined;
var clientPrivateKey = undefined;
var clientPublicKeyStr = undefined;

// Source - https://stackoverflow.com/a/15724300
// Posted by kirlich, modified by community. See post 'Timeline' for change history
// Retrieved 2026-03-23, License - CC BY-SA 4.0

function getCookie(name) {
    const value = `; ${document.cookie}`;
    const parts = value.split(`; ${name}=`);
    if (parts.length === 2) return parts.pop().split(';').shift();
}


async function generateKeys() {
    try {
        const keypair = await window.crypto.subtle.generateKey(
            {
                name: "RSA-OAEP",
                modulusLength: 2048,
                publicExponent: new Uint8Array([1, 0, 1]),
                hash: "SHA-256"
            },
            true,
            ["encrypt", "decrypt"]
        );

        const exportedPubkey = await window.crypto.subtle.exportKey(
            "spki",
            keypair.publicKey
        );

        clientPrivateKey = keypair.privateKey;
        clientPublicKeyStr = btoa(String.fromCharCode(...new Uint8Array(exportedPubkey)));
        document.cookie = `client-public-key=${clientPublicKeyStr}`;

        return Result.ok();
    } catch (err) {
        return Result.err("Failed to generate keys", err);
    }
}


async function importSpkiPublicKey() {
    try {
        // extract server-public-key from cookie
        const serverPublicKeyStr = getCookie("server-public-key");
        if (serverPublicKeyStr === undefined) return Result.err("server-public-key is undefined");

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

        // import key
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

        return Result.ok(null);
    } catch (err) {
        return Result.err("Failed to import server-public-key", err);
    }
}

async function encryptBody(body) {
    try {
        // make sure server public key is defined
        if (serverPublicKey === undefined) {
            let res = await importSpkiPublicKey();
            if (res.is_err) return res;
        }

        // encode body
        const encodedBody = new TextEncoder().encode(body);

        // encrypt buffer
        const encryptedBuffer = await window.crypto.subtle.encrypt(
            { name: "RSA-OAEP" },
            serverPublicKey,
            encodedBody
        );

        // format buffer for sending to server
        const encryptedBytes = new Uint8Array(encryptedBuffer);
        const encryptedBase64 = btoa(String.fromCharCode(...encryptedBytes));
        return Result.ok(encryptedBase64);
    } catch (err) {
        return Result.err("Failed to encrypt body", err);
    }
}


async function decryptBody(encryptedBodyB64) {
    try {
        if (encryptedBodyB64 === undefined) return Result.err("Cannot decrypt an undefined value");
        if (clientPrivateKey === undefined) {
            let res = await generateKeys();
            if (res.is_err) return res;
        }

        // base64 decode buffer
        const encryptedBuffer = Uint8Array.from(atob(encryptedBodyB64), c => c.charCodeAt(0));

        // decrypt
        const decryptedBuffer = await window.crypto.subtle.decrypt(
            {
                name: "RSA-OAEP"
            },
            clientPrivateKey,
            encryptedBuffer
        );

        // decode buffer to plaintext
        const decoder = new TextDecoder();
        const decryptedText = decoder.decode(decryptedBuffer);
        return Result.ok(decryptedText);
    } catch (err) {
        return Result.err("Failed to decrypt body", err);
    }
}



async function fetchEncrypted(path, body, method, headers) {
    try {
        // make sure unnecessary fields exists
        if (method === undefined) method = "POST";
        if (headers === undefined) headers = new Headers();

        console.log("body");
        console.log(body);

        // ensure js objects are handled correctly
        if (typeof body == "object") {
            body = JSON.stringify(body);
            headers.append('Content-Type', 'application/json')
        }

        console.log("body");
        console.log(body);
        


        // make sure keys exist
        if (clientPrivateKey === undefined || clientPublicKeyStr === undefined) {
            let res = generateKeys();
            if (res.is_err) return res;
        }

        // encrypt data
        const encryptedBodyRes = await encryptBody(body);
        if (encryptedBodyRes.is_err) return encryptedBodyRes;
        const encryptedBody = encryptedBodyRes.value;


        // fetch data
        let resp = await fetch(
            path,
            {
                method: "POST",
                headers: headers,
                body: encryptedBody,
            }
        )

        // if (resp.status == 412) return await fetchEncrypted
        if (resp.status != 200) return Result.err(resp);

        let respTxt = await resp.text();
        return await decryptBody(respTxt);
    } catch (err) {
        return Result.err("Error caught in fetchEncrypted", err);
    }
}



