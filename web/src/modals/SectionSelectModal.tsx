/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import {
    forwardRef,
    useEffect,
    useImperativeHandle,
    useRef,
    useState
} from "react";
import { Button, Form, Modal, ModalProps } from "react-daisyui";

import { createPortal } from "react-dom";
import SectionSelect from "../components/SectionSelect";

export type SectionSelectModalProps = ModalProps & {
    sections: Section[];
    onSectionChange?: (section?: Section) => void;
};

export const SectionSelectModal = forwardRef<
    HTMLDialogElement,
    SectionSelectModalProps
>(({ sections, ...props }, ref) => {
    const [section, setSection] = useState<Section>();

    useEffect(() => {
        if (!section) {
            setSection(sections[0]);
        }
    }, [section, sections]);

    const internalRef = useRef<HTMLDialogElement>(null);
    useImperativeHandle<HTMLDialogElement | null, HTMLDialogElement | null>(
        ref,
        () => internalRef.current
    );

    return createPortal(
        <Modal ref={internalRef} {...props}>
            <h1 className="text-3xl font-bold">Select section</h1>
            <Form>
                <Form.Label title="Source section" />
                <SectionSelect
                    selectedSection={section}
                    sections={sections}
                    onSelected={(value) => {
                        setSection(value);
                    }}
                />
            </Form>

            <Modal.Actions>
                <Button
                    type="button"
                    color="primary"
                    onClick={(_e) => {
                        console.log("SectionSelectModal", section);
                        props.onSectionChange?.(section);
                    }}
                >
                    Select
                </Button>
            </Modal.Actions>
        </Modal>,
        document.body
    );
});
export default SectionSelectModal;
