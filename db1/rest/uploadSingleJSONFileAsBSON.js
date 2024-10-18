// ----------------------------------------------------------------------
// -- uploads and downloads a single JSON file as BLOB
// -- invocation:
// ----------------------------------------------------------------------

const express = require('express');
const { MongoClient, Binary } = require('mongodb');
const multer = require('multer');
const fs = require('fs');
const app = express();

// MongoDB connection URL and database details
const url = 'mongodb://localhost:27017';
const dbName = 'mydb';
const client = new MongoClient(url);

// Multer setup to handle file uploads
const upload = multer({ dest: 'uploads/' });

app.post('/upload', upload.single('file'), async (req, res) => {
    try {
        await client.connect();
        const db = client.db("mu3e");
        const collection = db.collection('detconfigs');

        const { tag, filename } = req.body; // Get 'tag' and 'filename' from form data
        const filePath = req.file.path; // Temporary path of the uploaded file

        // Read the file contents as a BLOB (binary data)
        const fileContent = fs.readFileSync(filePath);

        // Insert the document into MongoDB as a BLOB (Binary)
        await collection.insertOne({
            tag,
            filename,
            content: new Binary(fileContent) // Store file content as Binary
        });

        // Clean up the temporary file
        fs.unlinkSync(filePath);

        res.status(200).send('File uploaded successfully');
    } catch (error) {
        console.error(error);
        res.status(500).send('Error uploading file');
    } finally {
        await client.close();
    }
});

app.get('/download/:tag', async (req, res) => {
    try {
        await client.connect();
        const db = client.db("mu3e");
        const collection = db.collection("detconfigs");

        // Find the document with the given tag
        const fileDocument = await collection.findOne({ tag: req.params.tag });

        if (fileDocument) {
            // The file content is stored as a BLOB (Binary), so convert it back to JSON
            const fileContentBuffer = fileDocument.content.buffer;
            const fileContentJson = JSON.parse(fileContentBuffer.toString('utf8'));

            // Send the parsed JSON content as the response
            res.json(fileContentJson);
        } else {
            res.status(404).send('File not found');
        }
    } catch (error) {
        console.error(error);
        res.status(500).send('Error retrieving file');
    } finally {
        await client.close();
    }
});


app.listen(3000, () => {
    console.log('Server is running on port 3000');
});


