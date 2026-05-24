'use strict';

let hideUnknownMaps = false;
const hiddenColumns = new Set();
const collectionCounts = {};
let detailSequence = 0;

// unknown-map visibility
const dynamicStyle = document.createElement('style');
document.head.appendChild(dynamicStyle);

function updateDynamicStyles() {
    const rules = [];
    hiddenColumns.forEach(col => rules.push(`[data-col="${col}"] { display: none !important; }`));
    if (hideUnknownMaps) rules.push('.beatmap-row[data-unknown] { display: none !important; }');
    dynamicStyle.textContent = rules.join('\n');
}

function setColumnVisible(col, visible) {
    if (!visible) hiddenColumns.add(col); else hiddenColumns.delete(col);
    updateDynamicStyles();
}

function toggleHideUnknown() {
    hideUnknownMaps = !hideUnknownMaps;
    document.getElementById('hide-unknown-item').textContent =
        hideUnknownMaps ? 'Show Unknown Maps' : 'Hide Unknown Maps';
    updateDynamicStyles();
}

// Chunked renderer
const scheduleIdle = typeof requestIdleCallback !== 'undefined'
    ? fn => requestIdleCallback(fn, { timeout: 150 })
    : fn => setTimeout(fn, 0);

const RENDER_CHUNK = 150;
const listRenderGen = {};

function renderChunk(list, listId, items, start, gen) {
    if (listRenderGen[listId] !== gen) return;
    const end = Math.min(start + RENDER_CHUNK, items.length);
    const frag = document.createDocumentFragment();
    for (let i = start; i < end; i++) frag.appendChild(buildBeatmapRow(items[i]));
    list.appendChild(frag);
    if (end < items.length) scheduleIdle(() => renderChunk(list, listId, items, end, gen));
}

function startRender(list, listId) {
    const gen = (listRenderGen[listId] = (listRenderGen[listId] || 0) + 1);
    list.querySelectorAll('.beatmap-row').forEach(r => r.remove());
    renderChunk(list, listId, listData[listId].sorted, 0, gen);
}

const listData = {};

function setStatus(text) {
    document.getElementById('status-text').textContent = text;
}

function updateStatusCounts() {
    document.getElementById('status-counts').textContent =
        Object.entries(collectionCounts)
            .map(([mode, count]) => mode + ': ' + count + ' collections')
            .join('  ');
}

function showAbout() {
    const dialog = document.getElementById('about-dialog');
    dialog.style.transform = 'none';
    dialog.classList.remove('d-none');
    dialog.style.left = Math.max(0, (window.innerWidth - dialog.offsetWidth) / 2) + 'px';
    dialog.style.top = Math.max(0, (window.innerHeight - dialog.offsetHeight) / 3) + 'px';
}

(function initAboutDrag() {
    const dialog = document.getElementById('about-dialog');
    const titlebar = document.getElementById('about-titlebar');
    titlebar.addEventListener('mousedown', function(event) {
        event.preventDefault();
        const rect = dialog.getBoundingClientRect();
        dialog.style.transform = 'none';
        dialog.style.left = rect.left + 'px';
        dialog.style.top = rect.top + 'px';
        const originX = event.clientX - rect.left;
        const originY = event.clientY - rect.top;
        document.onmousemove = function(e) {
            const maxX = window.innerWidth - dialog.offsetWidth;
            const maxY = window.innerHeight - dialog.offsetHeight;
            dialog.style.left = Math.min(Math.max(0, e.clientX - originX), maxX) + 'px';
            dialog.style.top = Math.min(Math.max(0, e.clientY - originY), maxY) + 'px';
        };
        document.onmouseup = function() {
            document.onmousemove = null;
            document.onmouseup = null;
        };
    });
})();

// Helpers
function escapeHtmlKeepLeadingSpaces(str) {
    return (str || '')
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/^ +/, spaces => '&nbsp;'.repeat(spaces.length));
}

function formatStars(rating) {
    return rating > 0 ? rating.toFixed(2) + '\u2605' : '';
}

function cloneTemplate(id) {
    return document.getElementById(id).content.cloneNode(true).firstElementChild;
}

function switchTab(mode) {
    const button = document.getElementById('tab-' + mode);
    if (button) bootstrap.Tab.getOrCreateInstance(button).show();
}

// Column definitions
const columnDefs = [
    { name: 'Title', col: 'title', hideable: false },
    { name: 'Artist', col: 'artist', hideable: true },
    { name: 'Difficulty', col: 'difficulty', hideable: true },
    { name: 'Mapper', col: 'mapper', hideable: true },
    { name: 'Stars', col: 'stars', hideable: true },
    { name: 'ID', col: 'id', hideable: true },
    { name: 'Set', col: 'setid', hideable: true },
    { name: 'MD5', col: 'md5', hideable: true, extraClass: 'col-md5' },
];

// Column context menu
function showColumnMenu(event) {
    event.preventDefault();
    const menu = document.getElementById('col-context-menu');
    menu.innerHTML = '';
    columnDefs.forEach(def => {
        const item = document.createElement('div');
        item.className = 'col-context-item' + (!def.hideable ? ' disabled' : '');
        const checkbox = document.createElement('input');
        checkbox.type = 'checkbox';
        checkbox.checked = !hiddenColumns.has(def.col);
        checkbox.disabled = !def.hideable;
        const label = document.createElement('span');
        label.textContent = def.name;
        item.appendChild(checkbox);
        item.appendChild(label);
        if (def.hideable) {
            item.addEventListener('click', () => {
                const visible = hiddenColumns.has(def.col);
                checkbox.checked = visible;
                setColumnVisible(def.col, visible);
            });
        }
        menu.appendChild(item);
    });
    menu.style.left = Math.min(event.clientX, window.innerWidth - 180) + 'px';
    menu.style.top = Math.min(event.clientY, window.innerHeight - menu.offsetHeight - 8) + 'px';
    menu.classList.remove('d-none');
}

document.addEventListener('click', () => {
    document.getElementById('col-context-menu').classList.add('d-none');
});

// API actions
async function reloadAll() {
    setStatus('Reloading...');
    await fetch('/api/reload', { method: 'POST' });
    await Promise.all([loadCollections('stable'), loadCollections('lazer')]);
    setStatus('Ready');
}

async function recompileScss() {
    setStatus('Recompiling SCSS...');
    try {
        const response = await fetch('/api/recompile-scss', { method: 'POST' });
        const data = await response.json();
        if (data.status === 'ok') {
            setStatus('SCSS recompiled, reload the page to apply changes.');
        } else {
            setStatus('SCSS error: ' + (data.message || 'unknown error'));
        }
    } catch (error) {
        setStatus('SCSS error: ' + error.message);
    }
}

function sortList(listId, columnIndex) {
    const list = document.getElementById(listId);
    const state = listData[listId];
    if (!list || !state) return;

    const prevCol = state.sortCol;
    const prevDir = state.sortDir;
    let dir;
    if (prevCol !== columnIndex) dir = 1;
    else if (prevDir === 1) dir = -1;
    else if (prevDir === -1) dir = 0;
    else dir = 1;

    state.sortCol = columnIndex;
    state.sortDir = dir;

    list.querySelectorAll('.beatmap-header .beatmap-col').forEach((h, i) => {
        h.textContent = h.textContent.replace(/ [▲▼]$/, '');
        if (i === columnIndex && dir !== 0)
            h.textContent += dir === 1 ? ' ▲' : ' ▼';
    });

    const col = columnDefs[columnIndex]?.col;
    const isNumeric = col === 'stars' || col === 'id' || col === 'setid';

    state.sorted = dir === 0
        ? [...state.all]
        : [...state.all].sort((a, b) => dir * (isNumeric
            ? a._k[col] - b._k[col]
            : a._k[col].localeCompare(b._k[col])));

    startRender(list, listId);
}

function buildBeatmapRow(beatmap) {
    const notDownloaded = !beatmap.title;
    const row = cloneTemplate('tmpl-beatmap-row');
    if (notDownloaded) {
        row.classList.add('text-muted', 'fst-italic');
        row.dataset.unknown = '1';
    }

    const cells = {};
    row.querySelectorAll('[data-col]').forEach(el => { cells[el.dataset.col] = el; });

    columnDefs.forEach(def => {
        const cell = cells[def.col];
        if (!cell) return;
        if (def.extraClass) cell.classList.add(def.extraClass);

        switch (def.col) {
            case 'title': cell.textContent = notDownloaded ? 'not downloaded' : beatmap.title; break;
            case 'artist': cell.textContent = beatmap.artist || ''; break;
            case 'difficulty': cell.textContent = beatmap.difficulty || ''; break;
            case 'mapper': cell.textContent = beatmap.mapper || ''; break;
            case 'stars': cell.textContent = formatStars(beatmap.stars); break;
            case 'id':
                if (beatmap.id > 0) {
                    const a = document.createElement('a');
                    a.href = 'https://osu.ppy.sh/b/' + beatmap.id;
                    a.target = '_blank'; a.textContent = beatmap.id;
                    cell.appendChild(a);
                }
                break;
            case 'setid':
                if (beatmap.setId > 0) {
                    const a = document.createElement('a');
                    a.href = 'https://osu.ppy.sh/beatmapsets/' + beatmap.setId;
                    a.target = '_blank'; a.textContent = beatmap.setId;
                    cell.appendChild(a);
                }
                break;
            case 'md5': cell.textContent = beatmap.md5 || ''; break;
        }
    });
    return row;
}

function buildBeatmapList(collection, listId) {
    // Pre-compute lowercase sort keys so sort never touches the DOM
    const prepared = collection.beatmaps.map(b => ({
        ...b,
        _k: {
            title: (b.title || '').toLowerCase(),
            artist: (b.artist || '').toLowerCase(),
            difficulty: (b.difficulty || '').toLowerCase(),
            mapper: (b.mapper || '').toLowerCase(),
            stars: b.stars || 0,
            id: b.id || 0,
            setid: b.setId || 0,
            md5: (b.md5 || '').toLowerCase(),
        }
    }));
    listData[listId] = { all: prepared, sorted: prepared, sortCol: -1, sortDir: 0 };

    const list = cloneTemplate('tmpl-beatmap-list');
    list.id = listId;

    const header = cloneTemplate('tmpl-beatmap-header');
    columnDefs.forEach((def, index) => {
        const col = cloneTemplate('tmpl-beatmap-header-col');
        col.textContent = def.name;
        col.dataset.col = def.col;
        if (def.extraClass) col.classList.add(def.extraClass);
        col.addEventListener('click', () => sortList(listId, index));
        header.appendChild(col);
    });
    list.appendChild(header);

    // First RENDER_CHUNK rows rendered synchronously
    startRender(list, listId);

    return list;
}

function renderBeatmaps(collection) {
    const fragment = document.createDocumentFragment();
    if (!collection.beatmaps || collection.beatmaps.length === 0) {
        const p = document.createElement('p');
        p.className = 'text-muted p-2 m-0';
        p.innerHTML = escapeHtmlKeepLeadingSpaces(collection.name) + ' - no beatmaps';
        fragment.appendChild(p);
        return fragment;
    }
    const listId = 'lst-' + (++detailSequence);

    const header = cloneTemplate('tmpl-collection-header');
    header.querySelector('[data-slot="name"]').innerHTML = escapeHtmlKeepLeadingSpaces(collection.name);
    header.querySelector('[data-slot="count"]').textContent = collection.beatmaps.length + ' Maps';
    fragment.appendChild(header);
    fragment.appendChild(buildBeatmapList(collection, listId));

    return fragment;
}

async function loadCollections(mode) {
    const listElement = document.getElementById(mode + '-list');
    try {
        const response = await fetch('/api/' + mode + '/collections');
        if (!response.ok) throw new Error(await response.text());
        const collections = await response.json();

        const label = mode[0].toUpperCase() + mode.slice(1);
        collectionCounts[label] = collections.length;
        updateStatusCounts();

        if (collections.length === 0) {
            const div = document.createElement('div');
            div.className = 'list-group-item text-muted py-1 border-0 fst-italic';
            div.textContent = 'No collections';
            listElement.replaceChildren(div);
            return;
        }

        const fragment = document.createDocumentFragment();
        collections.forEach(collection => {
            const item = cloneTemplate('tmpl-collection-item');
            item.dataset.name = collection.name || '';
            item.querySelector('[data-slot="name"]').innerHTML =
                escapeHtmlKeepLeadingSpaces(collection.name) || '<em class="text-muted">(unnamed)</em>';
            item.querySelector('[data-slot="count"]').textContent = collection.count;
            item.addEventListener('click', () => selectCollection(mode, item));
            fragment.appendChild(item);
        });
        listElement.replaceChildren(fragment);

    } catch (error) {
        const div = document.createElement('div');
        div.className = 'list-group-item text-danger py-1 border-0';
        div.textContent = error.message;
        listElement.replaceChildren(div);
        setStatus('Error: ' + error.message);
    }
}

async function selectCollection(mode, element) {
    document.querySelectorAll('#' + mode + '-list .list-group-item')
        .forEach(item => item.classList.remove('active'));
    element.classList.add('active');

    const name = element.dataset.name;
    const detailPanel = document.getElementById(mode + '-detail');
    const loading = document.createElement('p');
    loading.className = 'text-muted p-2 m-0';
    loading.textContent = 'Loading...';
    detailPanel.replaceChildren(loading);
    document.getElementById('status-text').innerHTML = 'Loading "' + escapeHtmlKeepLeadingSpaces(name) + '"...';

    try {
        const response = await fetch('/api/' + mode + '/collections?name=' + encodeURIComponent(name));
        if (!response.ok) throw new Error(await response.text());
        const collection = await response.json();
        detailPanel.replaceChildren(renderBeatmaps(collection));
        document.getElementById('status-text').innerHTML =
            '"' + escapeHtmlKeepLeadingSpaces(name) + '" (' + (collection.beatmaps ? collection.beatmaps.length : 0) + ' beatmaps)';
    } catch (error) {
        const div = document.createElement('div');
        div.className = 'alert alert-danger m-2 py-1 rounded-0';
        div.textContent = error.message;
        detailPanel.replaceChildren(div);
        setStatus('Error: ' + error.message);
    }
}

loadCollections('stable');
loadCollections('lazer');
