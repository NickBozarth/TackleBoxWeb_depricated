const newTackleButton = document.querySelector("#new-tackle-button button");
const newTackleForm = document.getElementById("new-tackle-form");
const tackleTypeSelect = newTackleForm.querySelector("form select");
const tackleDataDiv = newTackleForm.querySelector("form div#data");

var tackleTypes = undefined;
var aesKey = undefined;

async function getAesKey(keyString) {
    if (aesKey !== undefined) return Result.ok(aesKey);

    const encoder = new TextEncoder();

    if (loginHash === undefined) return Result.err("You must be logged in to submit a tackle");
    const salt = encoder.encode(loginHash).subarray(0, 12);

    const keyBuffer = new TextEncoder().encode(keyString);

    let baseKey;

    try {
        baseKey = await window.crypto.subtle.importKey(
            "raw",
            keyBuffer,
            "PBKDF2",
            false,
            ["deriveKey"]
        );
    }
    catch (e) {
        return Result.err("Failed to import key", e);
    }

    try {
        aesKey = await window.crypto.subtle.deriveKey(
            {
                name: "PBKDF2",
                salt: salt,
                iterations: 600000,
                hash: "SHA-256"
            },
            baseKey,
            { name: "AES-GCM", length: 256 },
            true,
            ["encrypt", "decrypt"]
        );
    }
    catch (e) {
        return Result.err("Failed to derive key", e);
    }

    return Result.ok(aesKey);
}


async function aesEncrypt(unencrypted_object, keyString) {
    const plaintext = JSON.stringify(unencrypted_object);

    const textbuffer = new TextEncoder().encode(plaintext);

    let key = await getAesKey(keyString);
    if (key.is_err) return key;
    key = key.value;
    
    const iv = window.crypto.getRandomValues(new Uint8Array(12));

    let ciphertext;

    try {
        ciphertext = await window.crypto.subtle.encrypt(
            { name: "AES-GCM", iv: iv },
            key,
            textbuffer
        )
    }
    catch (e) {
        return Result.err("Failed to encrypt buffer", e);
    }

    const b64Ciphertext = btoa(String.fromCharCode(...new Uint8Array(ciphertext)));
    const b64Iv = btoa(String.fromCharCode(...iv));

    return Result.ok({ encrypted_buffer: b64Ciphertext, iv: b64Iv });
}




async function processNewTackle() {
    if (passwordHash === undefined || sessionId === undefined) 
        return Result.err("You must be logged in to submit a new tackle");

    const nameElement  = newTackleForm.querySelector("[name=name]");
    const descElement  = newTackleForm.querySelector("[name=desc]");
    const typeElement  = newTackleForm.querySelector("[name=type]");
    const textElement  = newTackleForm.querySelector("[name=text]");
    const filesElement = newTackleForm.querySelector("[name=file]");

    if (nameElement  === undefined) return Result.err("Failed to get name element");
    if (descElement  === undefined) return Result.err("Failed to get desc element");
    if (typeElement  === undefined) return Result.err("Failed to get type element");
    if (textElement  === undefined) return Result.err("Failed to get text element");
    if (filesElement === undefined) return Result.err("Failed to get files element");

    const name  = nameElement.value;
    const desc  = descElement.value;
    const type  = typeElement.value;
    const text  = textElement.value;
    // const files = filesElement.files;

    let files = [];
    for (file of filesElement.files) {
        files.push(await file.bytes());
    }

    let obj = {}
    obj["name"]  = name;
    if (descElement.hidden) obj["desc"]  = desc;
    if (typeElement.hidden) obj["type"]  = type;
    if (textElement.hidden) obj["text"]  = text;
    obj["files"] = files;

    console.log(obj);



    return Result.ok(obj);
}


async function getTackleSections() {
    return await fetch(
        "/sections",
        {
            method: "GET"
        }
        )
        .then(async response => {
            if (!response.ok) return Result.err("Response returned with code " + response.status);
            try {
                return Result.ok(await response.json());
            } catch {
                return Result.err("Error occurred when parsing response to json");
            }
        });
}


function showCorrectDataField() {
    const selectedOption = tackleTypeSelect.selectedOptions[0];
    if (selectedOption === undefined) return Result.err("No option selected");
    const selectedOptionTypes = selectedOption.classList;
    if (selectedOptionTypes === undefined) return Result.err("Selected option type not found");
    
    for (const child of tackleDataDiv.children) child.hidden = true;
    for (const selectedOptionType of selectedOptionTypes) {
        for (const child of tackleDataDiv.querySelectorAll(`.${selectedOptionType}`)) child.hidden = false;
    }
}


newTackleForm.addEventListener("submit", async event => {
    event.preventDefault();

    console.log(await processNewTackle());
});


newTackleButton.addEventListener("click", async event => {
    if (sessionId === undefined) return;
    if (tackleTypes === undefined) {
        let res = await getTackleSections();
        if (res.is_ok) tackleTypes = res.value;
        else return;
    }

    newTackleButton.hidden = true;
    newTackleForm.hidden = false;

    tackleTypes.forEach(type => {
        console.log(type);
        let newOption = new Option(type.type_name, type.type_name.toLowerCase());
        for(const className of type.class_list.split(" ")) newOption.classList.add(className);
        tackleTypeSelect.add(newOption); 
    });

    showCorrectDataField();
});

tackleTypeSelect.addEventListener("change", event => {
    showCorrectDataField();
});